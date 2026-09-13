#include <axiom/assets/AssetTypes.h>
#include <gtest/gtest.h>

using namespace axiom;

static MaterialAsset MakeMaterial() {
    MaterialAsset material;
    material.name = "round-trip";
    material.albedoColor = {0.1f, 0.2f, 0.3f, 0.4f};
    material.metallic = 0.5f;
    material.roughness = 0.75f;
    material.albedoTextureId = TypedUUID(1, 2, AssetTypeId::Texture);
    material.normalTextureId = TypedUUID(3, 4, AssetTypeId::Texture);
    material.metallicTextureId = TypedUUID(5, 6, AssetTypeId::Texture);
    return material;
}

static void ExpectEqual(const MaterialAsset &left, const MaterialAsset &right) {
    EXPECT_EQ(left.name, right.name);
    EXPECT_EQ(left.albedoColor, right.albedoColor);
    EXPECT_EQ(left.metallic, right.metallic);
    EXPECT_EQ(left.roughness, right.roughness);
    EXPECT_EQ(left.albedoTextureId, right.albedoTextureId);
    EXPECT_EQ(left.normalTextureId, right.normalTextureId);
    EXPECT_EQ(left.metallicTextureId, right.metallicTextureId);
}

TEST(MaterialAssetTest, JsonAndBinaryRoundTrip) {
    const MaterialAsset source = MakeMaterial();

    JsonSerializer jsonWriter;
    auto jsonValue = source;
    jsonValue.Serialize(jsonWriter);
    JsonSerializer jsonReader;
    ASSERT_TRUE(jsonReader.LoadFromString(jsonWriter.SaveToString(false)));
    MaterialAsset jsonResult;
    jsonResult.Serialize(jsonReader);
    ExpectEqual(source, jsonResult);

    BinarySerializer binaryWriter;
    auto binaryValue = source;
    binaryValue.Serialize(binaryWriter);
    BinarySerializer binaryReader;
    ASSERT_TRUE(binaryReader.LoadFromBytes(binaryWriter.SaveToBytes()));
    MaterialAsset binaryResult;
    binaryResult.Serialize(binaryReader);
    ExpectEqual(source, binaryResult);
}
