// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "CharacterMaterialSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"

UCharacterMaterialSystem::UCharacterMaterialSystem()
{
    // Initialize default material parameters
    DefaultSkinRoughness = 0.8f;
    DefaultSkinSpecular = 0.3f;
    DefaultSkinSubsurface = 0.5f;
    
    bMaterialsInitialized = false;
}

void UCharacterMaterialSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    LoadBaseMaterials();
    InitializeMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Character Material System initialized"));
}

void UCharacterMaterialSystem::Deinitialize()
{
    // Clear all cached materials
    SkinMaterials.Empty();
    ClothingMaterials.Empty();
    HairMaterials.Empty();
    CachedDynamicMaterials.Empty();
    
    Super::Deinitialize();
}

void UCharacterMaterialSystem::LoadBaseMaterials()
{
    // Load base skin materials for different skin tones
    LoadSkinMaterials();
    LoadClothingMaterials();
    LoadHairMaterials();
    
    bMaterialsInitialized = true;
}

void UCharacterMaterialSystem::LoadSkinMaterials()
{
    // Initialize skin material paths - these would be actual MetaHuman materials
    TMap<ESkinTone, FString> SkinMaterialPaths = {
        {ESkinTone::VeryPale, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_VeryPale")},
        {ESkinTone::Pale, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Pale")},
        {ESkinTone::Light, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Light")},
        {ESkinTone::Medium, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Medium")},
        {ESkinTone::Olive, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Olive")},
        {ESkinTone::Tan, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Tan")},
        {ESkinTone::Dark, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_Dark")},
        {ESkinTone::VeryDark, TEXT("/Game/MetaHumans/Common/Materials/M_Skin_VeryDark")}
    };
    
    for (const auto& MaterialPair : SkinMaterialPaths)
    {
        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPair.Value);
        if (Material)
        {
            SkinMaterials.Add(MaterialPair.Key, Material);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load skin material: %s"), *MaterialPair.Value);
        }
    }
}

void UCharacterMaterialSystem::LoadClothingMaterials()
{
    // Load base clothing materials
    TArray<FString> ClothingMaterialPaths = {
        TEXT("/Game/TranspersonalGame/Materials/Clothing/M_Fabric_Cotton"),
        TEXT("/Game/TranspersonalGame/Materials/Clothing/M_Fabric_Linen"),
        TEXT("/Game/TranspersonalGame/Materials/Clothing/M_Fabric_Wool"),
        TEXT("/Game/TranspersonalGame/Materials/Clothing/M_Leather_Base"),
        TEXT("/Game/TranspersonalGame/Materials/Clothing/M_Metal_Buckles")
    };
    
    for (const FString& MaterialPath : ClothingMaterialPaths)
    {
        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *MaterialPath);
        if (Material)
        {
            ClothingMaterials.Add(Material);
        }
    }
}

void UCharacterMaterialSystem::LoadHairMaterials()
{
    // Load hair materials for different colors
    TMap<FLinearColor, FString> HairColorPaths = {
        {FLinearColor::Black, TEXT("/Game/MetaHumans/Common/Materials/M_Hair_Black")},
        {FLinearColor(0.3f, 0.15f, 0.05f), TEXT("/Game/MetaHumans/Common/Materials/M_Hair_Brown")},
        {FLinearColor(0.8f, 0.6f, 0.2f), TEXT("/Game/MetaHumans/Common/Materials/M_Hair_Blonde")},
        {FLinearColor(0.6f, 0.2f, 0.1f), TEXT("/Game/MetaHumans/Common/Materials/M_Hair_Red")},
        {FLinearColor(0.5f, 0.5f, 0.5f), TEXT("/Game/MetaHumans/Common/Materials/M_Hair_Gray")}
    };
    
    for (const auto& HairPair : HairColorPaths)
    {
        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, *HairPair.Value);
        if (Material)
        {
            HairMaterials.Add(HairPair.Key, Material);
        }
    }
}

void UCharacterMaterialSystem::InitializeMaterialParameters()
{
    // Set up default material parameter collections
    SkinParameterNames.Add(TEXT("BaseColor"));
    SkinParameterNames.Add(TEXT("Roughness"));
    SkinParameterNames.Add(TEXT("Specular"));
    SkinParameterNames.Add(TEXT("SubsurfaceColor"));
    SkinParameterNames.Add(TEXT("SubsurfaceRadius"));
    SkinParameterNames.Add(TEXT("Opacity"));
    
    ClothingParameterNames.Add(TEXT("BaseColor"));
    ClothingParameterNames.Add(TEXT("Roughness"));
    ClothingParameterNames.Add(TEXT("Metallic"));
    ClothingParameterNames.Add(TEXT("DirtLevel"));
    ClothingParameterNames.Add(TEXT("WearLevel"));
    ClothingParameterNames.Add(TEXT("Wetness"));
}

UMaterialInstanceDynamic* UCharacterMaterialSystem::CreateSkinMaterial(ESkinTone SkinTone, const FCharacterVisualStory& VisualStory)
{
    UMaterialInterface* BaseMaterial = SkinMaterials.FindRef(SkinTone);
    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("No base material found for skin tone"));
        return nullptr;
    }
    
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!DynamicMaterial)
    {
        return nullptr;
    }
    
    // Apply visual story effects to skin
    ApplySurvivalEffectsToSkin(DynamicMaterial, VisualStory);
    
    // Cache the material
    CachedDynamicMaterials.Add(DynamicMaterial);
    
    return DynamicMaterial;
}

UMaterialInstanceDynamic* UCharacterMaterialSystem::CreateClothingMaterial(EClothingType ClothingType, const FCharacterVisualStory& VisualStory)
{
    if (ClothingMaterials.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("No clothing materials loaded"));
        return nullptr;
    }
    
    // Select appropriate base material based on clothing type
    int32 MaterialIndex = static_cast<int32>(ClothingType) % ClothingMaterials.Num();
    UMaterialInterface* BaseMaterial = ClothingMaterials[MaterialIndex];
    
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!DynamicMaterial)
    {
        return nullptr;
    }
    
    // Apply wear and environmental effects
    ApplyClothingWear(DynamicMaterial, VisualStory);
    
    CachedDynamicMaterials.Add(DynamicMaterial);
    
    return DynamicMaterial;
}

UMaterialInstanceDynamic* UCharacterMaterialSystem::CreateHairMaterial(const FLinearColor& HairColor, const FCharacterVisualStory& VisualStory)
{
    // Find closest hair color match
    UMaterialInterface* BaseMaterial = FindClosestHairMaterial(HairColor);
    if (!BaseMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("No hair material found"));
        return nullptr;
    }
    
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (!DynamicMaterial)
    {
        return nullptr;
    }
    
    // Apply custom hair color
    DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), HairColor);
    
    // Apply environmental effects (sun bleaching, dirt, etc.)
    ApplyHairEnvironmentalEffects(DynamicMaterial, VisualStory);
    
    CachedDynamicMaterials.Add(DynamicMaterial);
    
    return DynamicMaterial;
}

void UCharacterMaterialSystem::ApplyCharacterMaterials(ACharacter* Character, const FMetaHumanCustomization& Customization)
{
    if (!Character || !bMaterialsInitialized)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (!MeshComponent)
    {
        return;
    }
    
    // Create and apply skin material
    UMaterialInstanceDynamic* SkinMaterial = CreateSkinMaterial(Customization.SkinTone, Customization.VisualStory);
    if (SkinMaterial)
    {
        // Apply to skin material slots (typically 0-2 for MetaHumans)
        MeshComponent->SetMaterial(0, SkinMaterial); // Head
        MeshComponent->SetMaterial(1, SkinMaterial); // Body
        MeshComponent->SetMaterial(2, SkinMaterial); // Arms/Legs
    }
    
    // Create and apply clothing materials
    ApplyClothingMaterials(Character, Customization.VisualStory);
    
    // Create and apply hair material
    UMaterialInstanceDynamic* HairMaterial = CreateHairMaterial(Customization.HairColor, Customization.VisualStory);
    if (HairMaterial)
    {
        // Apply to hair material slots
        MeshComponent->SetMaterial(3, HairMaterial); // Hair
        MeshComponent->SetMaterial(4, HairMaterial); // Facial Hair
    }
}

void UCharacterMaterialSystem::ApplySurvivalEffectsToSkin(UMaterialInstanceDynamic* SkinMaterial, const FCharacterVisualStory& VisualStory)
{
    if (!SkinMaterial)
    {
        return;
    }
    
    // Apply sun exposure effects
    float SunDamage = FMath::Clamp(VisualStory.SunExposure, 0.0f, 1.0f);
    SkinMaterial->SetScalarParameterValue(TEXT("SunDamage"), SunDamage);
    
    // Apply dirt and grime
    float DirtLevel = FMath::Clamp(VisualStory.DirtAccumulation, 0.0f, 1.0f);
    SkinMaterial->SetScalarParameterValue(TEXT("DirtLevel"), DirtLevel);
    
    // Apply health effects
    float HealthMultiplier = FMath::Clamp(VisualStory.HealthCondition, 0.1f, 1.0f);
    SkinMaterial->SetScalarParameterValue(TEXT("HealthMultiplier"), HealthMultiplier);
    
    // Apply stress effects (affects skin tone and texture)
    float StressLevel = FMath::Clamp(VisualStory.StressLevel, 0.0f, 1.0f);
    SkinMaterial->SetScalarParameterValue(TEXT("StressLevel"), StressLevel);
    
    // Apply starvation effects if present
    if (VisualStory.bShowsSignsOfStarvation)
    {
        SkinMaterial->SetScalarParameterValue(TEXT("Emaciation"), 0.7f);
        SkinMaterial->SetVectorParameterValue(TEXT("UnhealthyTint"), FLinearColor(0.8f, 0.9f, 0.7f, 1.0f));
    }
}

void UCharacterMaterialSystem::ApplyClothingWear(UMaterialInstanceDynamic* ClothingMaterial, const FCharacterVisualStory& VisualStory)
{
    if (!ClothingMaterial)
    {
        return;
    }
    
    // Apply overall wear level
    float WearLevel = FMath::Clamp(VisualStory.OverallClothingWear, 0.0f, 1.0f);
    ClothingMaterial->SetScalarParameterValue(TEXT("WearLevel"), WearLevel);
    
    // Apply dirt accumulation
    float DirtLevel = FMath::Clamp(VisualStory.DirtAccumulation, 0.0f, 1.0f);
    ClothingMaterial->SetScalarParameterValue(TEXT("DirtLevel"), DirtLevel);
    
    // Apply environmental effects
    ClothingMaterial->SetScalarParameterValue(TEXT("SunFading"), VisualStory.SunExposure * 0.5f);
    ClothingMaterial->SetScalarParameterValue(TEXT("Wetness"), 0.0f); // Default dry
    
    // Apply damage based on survival time
    float DamageLevel = FMath::Clamp(VisualStory.SurvivalTime / 100.0f, 0.0f, 1.0f); // 100 days = max damage
    ClothingMaterial->SetScalarParameterValue(TEXT("DamageLevel"), DamageLevel);
}

void UCharacterMaterialSystem::ApplyHairEnvironmentalEffects(UMaterialInstanceDynamic* HairMaterial, const FCharacterVisualStory& VisualStory)
{
    if (!HairMaterial)
    {
        return;
    }
    
    // Apply sun bleaching
    float SunBleaching = VisualStory.SunExposure * 0.3f; // Hair bleaches less than skin burns
    HairMaterial->SetScalarParameterValue(TEXT("SunBleaching"), SunBleaching);
    
    // Apply dirt and oil buildup
    float DirtLevel = FMath::Clamp(VisualStory.DirtAccumulation, 0.0f, 1.0f);
    HairMaterial->SetScalarParameterValue(TEXT("DirtLevel"), DirtLevel);
    
    // Apply health effects (hair condition reflects overall health)
    float HealthMultiplier = FMath::Clamp(VisualStory.HealthCondition, 0.2f, 1.0f);
    HairMaterial->SetScalarParameterValue(TEXT("HealthMultiplier"), HealthMultiplier);
    
    // Stress can cause hair to become more brittle and dull
    float StressEffect = VisualStory.StressLevel * 0.4f;
    HairMaterial->SetScalarParameterValue(TEXT("StressDullness"), StressEffect);
}

void UCharacterMaterialSystem::ApplyClothingMaterials(ACharacter* Character, const FCharacterVisualStory& VisualStory)
{
    if (!Character)
    {
        return;
    }
    
    USkeletalMeshComponent* MeshComponent = Character->GetMesh();
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply different clothing materials to different slots
    // This assumes MetaHuman clothing setup with specific material slots
    
    // Shirt/Top (slot 5)
    UMaterialInstanceDynamic* ShirtMaterial = CreateClothingMaterial(EClothingType::Shirt, VisualStory);
    if (ShirtMaterial)
    {
        MeshComponent->SetMaterial(5, ShirtMaterial);
    }
    
    // Pants/Bottom (slot 6)
    UMaterialInstanceDynamic* PantsMaterial = CreateClothingMaterial(EClothingType::Pants, VisualStory);
    if (PantsMaterial)
    {
        MeshComponent->SetMaterial(6, PantsMaterial);
    }
    
    // Shoes (slot 7)
    UMaterialInstanceDynamic* ShoesMaterial = CreateClothingMaterial(EClothingType::Shoes, VisualStory);
    if (ShoesMaterial)
    {
        MeshComponent->SetMaterial(7, ShoesMaterial);
    }
}

UMaterialInterface* UCharacterMaterialSystem::FindClosestHairMaterial(const FLinearColor& TargetColor)
{
    UMaterialInterface* ClosestMaterial = nullptr;
    float ClosestDistance = FLT_MAX;
    
    for (const auto& HairPair : HairMaterials)
    {
        FLinearColor MaterialColor = HairPair.Key;
        float Distance = FVector::Dist(\n            FVector(TargetColor.R, TargetColor.G, TargetColor.B),\n            FVector(MaterialColor.R, MaterialColor.G, MaterialColor.B)\n        );\n        \n        if (Distance < ClosestDistance)\n        {\n            ClosestDistance = Distance;\n            ClosestMaterial = HairPair.Value;\n        }\n    }\n    \n    return ClosestMaterial;\n}\n\nvoid UCharacterMaterialSystem::UpdateMaterialForTimeOfDay(ACharacter* Character, float TimeOfDay)\n{\n    if (!Character)\n    {\n        return;\n    }\n    \n    USkeletalMeshComponent* MeshComponent = Character->GetMesh();\n    if (!MeshComponent)\n    {\n        return;\n    }\n    \n    // Update all dynamic materials for time of day lighting changes\n    for (int32 i = 0; i < MeshComponent->GetNumMaterials(); ++i)\n    {\n        UMaterialInstanceDynamic* DynamicMat = Cast<UMaterialInstanceDynamic>(MeshComponent->GetMaterial(i));\n        if (DynamicMat)\n        {\n            // Adjust material properties based on time of day\n            float LightingMultiplier = CalculateLightingMultiplier(TimeOfDay);\n            DynamicMat->SetScalarParameterValue(TEXT("LightingMultiplier"), LightingMultiplier);\n            \n            // Adjust subsurface scattering for skin materials\n            if (i <= 2) // Skin material slots\n            {\n                float SubsurfaceMultiplier = FMath::Lerp(0.3f, 1.0f, LightingMultiplier);\n                DynamicMat->SetScalarParameterValue(TEXT("SubsurfaceMultiplier"), SubsurfaceMultiplier);\n            }\n        }\n    }\n}\n\nfloat UCharacterMaterialSystem::CalculateLightingMultiplier(float TimeOfDay)\n{\n    // Convert time of day (0-24) to lighting multiplier (0-1)\n    // Dawn: 6-8, Day: 8-18, Dusk: 18-20, Night: 20-6\n    \n    if (TimeOfDay >= 8.0f && TimeOfDay <= 18.0f)\n    {\n        return 1.0f; // Full daylight\n    }\n    else if (TimeOfDay >= 6.0f && TimeOfDay < 8.0f)\n    {\n        return FMath::Lerp(0.2f, 1.0f, (TimeOfDay - 6.0f) / 2.0f); // Dawn\n    }\n    else if (TimeOfDay > 18.0f && TimeOfDay <= 20.0f)\n    {\n        return FMath::Lerp(1.0f, 0.2f, (TimeOfDay - 18.0f) / 2.0f); // Dusk\n    }\n    else\n    {\n        return 0.2f; // Night\n    }\n}