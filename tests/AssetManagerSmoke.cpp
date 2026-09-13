#include <axiom/assets/AssetManager.h>
#include <axiom/assets/AssetRegistry.h>
#include <axiom/assets/VFS.h>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace axiom;

class CountingLoader final : public IAssetLoader {
  public:
    std::atomic<int> loads{0};
    std::atomic<int> reloads{0};
    std::atomic<int> unloads{0};

    void *Load(const std::vector<uint8_t> &data, size_t &sizeBytes) override {
        ++loads;
        sizeBytes = data.size();
        auto *value = new int(data.empty() ? 0 : data.front());
        return value;
    }

    void *Reload(void *existing, const std::vector<uint8_t> &data,
                 size_t &sizeBytes) override {
        ++reloads;
        void *replacement = Load(data, sizeBytes);
        if (replacement && existing) Unload(existing);
        return replacement;
    }

    void Unload(void *data) override {
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

    VFS::Init();
    ASSERT_TRUE(VFS::Mount("smoke", root.string(), VFS::MountType::Directory, false));
    AssetManager::Init(0);
    CountingLoader loader;
    AssetManager::RegisterLoader(AssetTypeId::Texture, &loader);
    AssetManager::RegisterLoader(AssetTypeId::Mesh, &loader);

    const TypedUUID textureId(11, 1, AssetTypeId::Texture);
    {
        auto handle = AssetManager::LoadFromMemory<int>(textureId, {1});
        ASSERT_TRUE(handle.IsReady());
        ASSERT_EQ(*handle.Get(), 1);
    }
    EXPECT_EQ(loader.unloads, 1);
    EXPECT_EQ(AssetManager::GetLoadedAssetCount(), 0);
    {
        auto handle = AssetManager::LoadFromMemory<int>(textureId, {2});
        ASSERT_TRUE(handle.IsReady());
        ASSERT_EQ(*handle.Get(), 2);
    }
    EXPECT_EQ(loader.loads, 2);
    EXPECT_EQ(loader.unloads, 2);

    AssetRegistryEntry reloadEntry;
    reloadEntry.uuid = textureId;
    reloadEntry.physicalPath = "smoke://asset_manager_smoke.bin";
    AssetRegistry::Register(reloadEntry);
    auto reloadHandle = AssetManager::LoadFromMemory<int>(textureId, {3});
    AssetManager::Reload(reloadHandle);
    EXPECT_EQ(loader.reloads, 1);
    ASSERT_NE(reloadHandle.Get(), nullptr);
    EXPECT_EQ(*reloadHandle.Get(), 7);
    EXPECT_EQ(loader.unloads, 3);

    const TypedUUID parentId(12, 1, AssetTypeId::Mesh);
    const TypedUUID dependencyId(13, 1, AssetTypeId::Texture);
    AssetRegistryEntry parentEntry;
    parentEntry.uuid = parentId;
    parentEntry.dependencies.push_back(
        {dependencyId, AssetDependencyType::Hard, 0});
    AssetRegistry::Register(parentEntry);
    AssetRegistryEntry dependencyEntry;
    dependencyEntry.uuid = dependencyId;
    AssetRegistry::Register(dependencyEntry);

    auto parent = AssetManager::LoadFromMemory<void>(parentId, {4});
    auto dependency = AssetManager::LoadFromMemory<int>(dependencyId, {5});
    AssetManager::LoadDependencies(parent);
    dependency = AssetHandle<int>();
    const int unloadsBeforeParent = loader.unloads.load();
    EXPECT_EQ(unloadsBeforeParent, 3);
    parent = AssetHandle<void>();
    EXPECT_EQ(loader.unloads, unloadsBeforeParent + 2);

    reloadHandle = AssetHandle<int>();
    AssetManager::Shutdown();
    VFS::Unmount("smoke");
    VFS::Shutdown();
    std::filesystem::remove(reloadPath);
}
