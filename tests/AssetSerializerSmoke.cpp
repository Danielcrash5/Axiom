/**
 * @file tests/AssetSerializerSmoke.cpp
 * @brief Implements the asset serializer smoke module.
 */

#include <axiom/assets/AssetTypes.h>
#include <gtest/gtest.h>

using namespace axiom;

/**
 * @brief Performs the make material operation.
 * @return The result of the operation.
 */
static MaterialAsset makeMaterial() {
    MaterialAsset material;
    material.name = "round-trip";
    material.albedoColor = {0.1f, 0.2f, 0.3f, 0.4f};
    material.metallic = 0.5f;
    material.roughness = 0.75f;
    material.albedoTextureId = TypedUUID(1, 2, AssetTypeId::texture);
    material.normalTextureId = TypedUUID(3, 4, AssetTypeId::texture);
    material.metallicTextureId = TypedUUID(5, 6, AssetTypeId::texture);
    return material;
}

/**
 * @brief Performs the expect equal operation.
 * @param left The left used by this operation.
 * @param right The right used by this operation.
 */
static void expectEqual(const MaterialAsset &left, const MaterialAsset &right) {
    EXPECT_EQ(left.name, right.name);
    EXPECT_EQ(left.albedoColor, right.albedoColor);
    EXPECT_EQ(left.metallic, right.metallic);
    EXPECT_EQ(left.roughness, right.roughness);
    EXPECT_EQ(left.albedoTextureId, right.albedoTextureId);
    EXPECT_EQ(left.normalTextureId, right.normalTextureId);
    EXPECT_EQ(left.metallicTextureId, right.metallicTextureId);
}

TEST(MaterialAssetTest, JsonAndBinaryRoundTrip) {
    const MaterialAsset source = makeMaterial();

    JsonSerializer jsonWriter;
    auto jsonValue = source;
    jsonValue.serialize(jsonWriter);
    JsonSerializer jsonReader;
    ASSERT_TRUE(jsonReader.loadFromString(jsonWriter.saveToString(false)));
    MaterialAsset jsonResult;
    jsonResult.serialize(jsonReader);
    expectEqual(source, jsonResult);

    BinarySerializer binaryWriter;
    auto binaryValue = source;
    binaryValue.serialize(binaryWriter);
    BinarySerializer binaryReader;
    ASSERT_TRUE(binaryReader.loadFromBytes(binaryWriter.saveToBytes()));
    MaterialAsset binaryResult;
    binaryResult.serialize(binaryReader);
    expectEqual(source, binaryResult);
}
