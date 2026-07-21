#include "Char_PrimitiveHumanAsset.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"

UChar_PrimitiveHumanAsset::UChar_PrimitiveHumanAsset()
{
    // Initialize default presets
    HunterPreset.SkinTone = EChar_SkinTone::Medium;
    HunterPreset.BodyType = EChar_BodyType::Lean;
    HunterPreset.ClothingStyle = EChar_ClothingStyle::Minimal;
    HunterPreset.ScarIntensity = 0.2f;
    HunterPreset.WeatheringLevel = 0.6f;
    HunterPreset.bHasTattoos = false;
    HunterPreset.bHasBoneJewelry = false;
    HunterPreset.bCarriesStoneAxe = false;
    HunterPreset.bCarriesSpear = true;
    HunterPreset.bCarriesBow = true;

    WarriorPreset.SkinTone = EChar_SkinTone::Dark;
    WarriorPreset.BodyType = EChar_BodyType::Muscular;
    WarriorPreset.ClothingStyle = EChar_ClothingStyle::Decorated;
    WarriorPreset.ScarIntensity = 0.8f;
    WarriorPreset.WeatheringLevel = 0.7f;
    WarriorPreset.bHasTattoos = true;
    WarriorPreset.bHasBoneJewelry = true;
    WarriorPreset.bCarriesStoneAxe = true;
    WarriorPreset.bCarriesSpear = true;
    WarriorPreset.bCarriesBow = false;

    GathererPreset.SkinTone = EChar_SkinTone::Light;
    GathererPreset.BodyType = EChar_BodyType::Stocky;
    GathererPreset.ClothingStyle = EChar_ClothingStyle::Practical;
    GathererPreset.ScarIntensity = 0.1f;
    GathererPreset.WeatheringLevel = 0.4f;
    GathererPreset.bHasTattoos = false;
    GathererPreset.bHasBoneJewelry = true;
    GathererPreset.bCarriesStoneAxe = false;
    GathererPreset.bCarriesSpear = false;
    GathererPreset.bCarriesBow = false;

    ShamanPreset.SkinTone = EChar_SkinTone::Weathered;
    ShamanPreset.BodyType = EChar_BodyType::Tall;
    ShamanPreset.ClothingStyle = EChar_ClothingStyle::Ceremonial;
    ShamanPreset.ScarIntensity = 0.5f;
    ShamanPreset.WeatheringLevel = 0.9f;
    ShamanPreset.bHasTattoos = true;
    ShamanPreset.bHasBoneJewelry = true;
    ShamanPreset.bCarriesStoneAxe = false;
    ShamanPreset.bCarriesSpear = false;
    ShamanPreset.bCarriesBow = false;
}

void UChar_PrimitiveHumanAsset::ApplyCharacterSpec(USkeletalMeshComponent* MeshComponent, const FChar_PrimitiveHumanSpec& Spec)
{
    if (!MeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("UChar_PrimitiveHumanAsset::ApplyCharacterSpec - MeshComponent is null"));
        return;
    }

    // Apply base mesh if available
    if (BaseMesh.IsValid())
    {
        USkeletalMesh* LoadedMesh = BaseMesh.LoadSynchronous();
        if (LoadedMesh)
        {
            MeshComponent->SetSkeletalMesh(LoadedMesh);
        }
    }

    // Apply skin material based on skin tone
    UMaterialInterface* SkinMaterial = GetSkinMaterial(Spec.SkinTone);
    if (SkinMaterial)
    {
        MeshComponent->SetMaterial(0, SkinMaterial); // Assuming skin is material slot 0
    }

    // Apply clothing material based on clothing style
    UMaterialInterface* ClothingMaterial = GetClothingMaterial(Spec.ClothingStyle);
    if (ClothingMaterial)
    {
        MeshComponent->SetMaterial(1, ClothingMaterial); // Assuming clothing is material slot 1
    }

    // Set scalar parameters for weathering and scars
    UMaterialInstanceDynamic* DynamicMaterial = MeshComponent->CreateAndSetMaterialInstanceDynamic(0);
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), Spec.WeatheringLevel);
        DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), Spec.ScarIntensity);
        DynamicMaterial->SetScalarParameterValue(TEXT("HasTattoos"), Spec.bHasTattoos ? 1.0f : 0.0f);
    }

    // TODO: Attach equipment meshes based on specification
    // This would require socket attachment system for weapons and accessories
    
    UE_LOG(LogTemp, Log, TEXT("Applied character spec - SkinTone: %d, BodyType: %d, ClothingStyle: %d"), 
           (int32)Spec.SkinTone, (int32)Spec.BodyType, (int32)Spec.ClothingStyle);
}

FChar_PrimitiveHumanSpec UChar_PrimitiveHumanAsset::GenerateRandomSpec()
{
    FChar_PrimitiveHumanSpec RandomSpec;
    
    // Random skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 3);
    RandomSpec.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Random body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    RandomSpec.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Random clothing style
    int32 ClothingIndex = FMath::RandRange(0, 3);
    RandomSpec.ClothingStyle = static_cast<EChar_ClothingStyle>(ClothingIndex);
    
    // Random physical traits
    RandomSpec.ScarIntensity = FMath::RandRange(0.0f, 1.0f);
    RandomSpec.WeatheringLevel = FMath::RandRange(0.2f, 0.9f);
    RandomSpec.bHasTattoos = FMath::RandBool();
    RandomSpec.bHasBoneJewelry = FMath::RandBool();
    
    // Random equipment (ensure at least one weapon)
    RandomSpec.bCarriesStoneAxe = FMath::RandBool();
    RandomSpec.bCarriesSpear = FMath::RandBool();
    RandomSpec.bCarriesBow = FMath::RandBool();
    
    // Ensure at least one weapon
    if (!RandomSpec.bCarriesStoneAxe && !RandomSpec.bCarriesSpear && !RandomSpec.bCarriesBow)
    {
        int32 WeaponChoice = FMath::RandRange(0, 2);
        switch (WeaponChoice)
        {
        case 0: RandomSpec.bCarriesStoneAxe = true; break;
        case 1: RandomSpec.bCarriesSpear = true; break;
        case 2: RandomSpec.bCarriesBow = true; break;
        }
    }
    
    return RandomSpec;
}

UMaterialInterface* UChar_PrimitiveHumanAsset::GetSkinMaterial(EChar_SkinTone SkinTone)
{
    if (SkinMaterials.Contains(SkinTone))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = SkinMaterials[SkinTone];
        if (MaterialPtr.IsValid())
        {
            return MaterialPtr.LoadSynchronous();
        }
    }
    
    // Fallback to default material
    UE_LOG(LogTemp, Warning, TEXT("Skin material not found for tone: %d"), (int32)SkinTone);
    return nullptr;
}

UMaterialInterface* UChar_PrimitiveHumanAsset::GetClothingMaterial(EChar_ClothingStyle ClothingStyle)
{
    if (ClothingMaterials.Contains(ClothingStyle))
    {
        TSoftObjectPtr<UMaterialInterface> MaterialPtr = ClothingMaterials[ClothingStyle];
        if (MaterialPtr.IsValid())
        {
            return MaterialPtr.LoadSynchronous();
        }
    }
    
    // Fallback to default material
    UE_LOG(LogTemp, Warning, TEXT("Clothing material not found for style: %d"), (int32)ClothingStyle);
    return nullptr;
}