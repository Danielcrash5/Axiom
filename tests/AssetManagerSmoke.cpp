/**
 * @file tests/AssetManagerSmoke.cpp
 * @brief Implements the asset manager smoke module.
 */

#include <axiom/assets/AssetManager.h>
#include <axiom/assets/AssetRegistry.h>
#include <axiom/assets/VFS.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace axiom;

/**
 * @brief Provides the counting loader functionality.
 * @tparam CountingLoader Template type used by this type.
 */
class CountingLoader final : public IAssetLoader {
  public:
    /** @brief Stores the loads. */
    std::atomic<int> loads{0};
    /** @brief Stores the reloads. */
    std::atomic<int> reloads{0};
    /** @brief Stores the unloads. */
    std::atomic<int> unloads{0};

    /**
     * @brief Loads the requested resource.
     * @param data The data used by this operation.
     * @param sizeBytes The size bytes used by this operation.
     * @return The result of the operation.
     */
    void *load(const std::vector<uint8_t> &data, size_t &sizeBytes) override {
        ++loads;
        sizeBytes = data.size();
        auto *value = new int(data.empty() ? 0 : data.front());
        return value;
    }

    /**
     * @brief Reloads the requested resource.
     * @param existing The existing used by this operation.
     * @param data The data used by this operation.
     * @param sizeBytes The size bytes used by this operation.
     * @return The result of the operation.
     */
    void *reload(void *existing, const std::vector<uint8_t> &data,
                 size_t &sizeBytes) override {
        ++reloads;
        void *replacement = load(data, sizeBytes);
        if (replacement && existing) unload(existing);
        return replacement;
    }

    /**
     * @brief Releases the loaded resource.
     * @param data The data used by this operation.
     */
    void unload(void *data) override {
        ++unloads;
        delete static_cast<int *>(data);
    }
};

TEST(AssetManagerTest, UnloadReloadAndHardDependencies) {
    const auto root = std::filesystem::current_path();
    const auto reloadPath = root / "asset_manager_smoke.bin";
    {
        std::ofstream file(reloadPath, std::ios::binary);
        file.put(static_cast<char>(7));
    }

    VFS::init();
    ASSERT_TRUE(VFS::mount("smoke", root.string(), VFS::MountType::directory, false));
    AssetManager::init(0);
    CountingLoader loader;
    AssetManager::registerLoader(AssetTypeId::texture, &loader);
    AssetManager::registerLoader(AssetTypeId::mesh, &loader);

    const TypedUUID textureId(11, 1, AssetTypeId::texture);
    {
        auto handle = AssetManager::loadFromMemory<int>(textureId, {1});
        ASSERT_TRUE(handle.isReady());
        ASSERT_EQ(*handle.get(), 1);
    }
    EXPECT_EQ(loader.unloads, 1);
    EXPECT_EQ(AssetManager::getLoadedAssetCount(), 0);
    {
        auto handle = AssetManager::loadFromMemory<int>(textureId, {2});
        ASSERT_TRUE(handle.isReady());
        ASSERT_EQ(*handle.get(), 2);
    }
    EXPECT_EQ(loader.loads, 2);
    EXPECT_EQ(loader.unloads, 2);

    AssetRegistryEntry reloadEntry;
    reloadEntry.uuid = textureId;
    reloadEntry.physicalPath = "smoke://asset_manager_smoke.bin";
    AssetRegistry::registerAsset(reloadEntry);
    auto reloadHandle = AssetManager::loadFromMemory<int>(textureId, {3});
    AssetManager::reload(reloadHandle);
    EXPECT_EQ(loader.reloads, 1);
    ASSERT_NE(reloadHandle.get(), nullptr);
    EXPECT_EQ(*reloadHandle.get(), 7);
    EXPECT_EQ(loader.unloads, 3);

    const TypedUUID parentId(12, 1, AssetTypeId::mesh);
    const TypedUUID dependencyId(13, 1, AssetTypeId::texture);
    AssetRegistryEntry parentEntry;
    parentEntry.uuid = parentId;
    parentEntry.dependencies.push_back(
        {dependencyId, AssetDependencyType::hard, 0});
    AssetRegistry::registerAsset(parentEntry);
    AssetRegistryEntry dependencyEntry;
    dependencyEntry.uuid = dependencyId;
    AssetRegistry::registerAsset(dependencyEntry);

    auto parent = AssetManager::loadFromMemory<void>(parentId, {4});
    auto dependency = AssetManager::loadFromMemory<int>(dependencyId, {5});
    AssetManager::loadDependencies(parent);
    dependency = AssetHandle<int>();
    const int unloadsBeforeParent = loader.unloads.load();
    EXPECT_EQ(unloadsBeforeParent, 3);
    parent = AssetHandle<void>();
    EXPECT_EQ(loader.unloads, unloadsBeforeParent + 2);

    reloadHandle = AssetHandle<int>();
    AssetManager::shutdown();
    VFS::unmount("smoke");
    VFS::shutdown();
    std::filesystem::remove(reloadPath);
}
