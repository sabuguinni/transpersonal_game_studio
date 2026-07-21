#include "CharacterCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/Character.h"
#include "Engine/Engine.h"

UCharacterCustomizationComponent::UCharacterCustomizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CachedMeshComponent = nullptr;
}

void UCharacterCustomizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache the skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        CachedMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CachedMeshComponent)
        {
            ApplyCustomization();
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("CharacterCustomizationComponent: No SkeletalMeshComponent found on owner"));
        }
    }
}

void UCharacterCustomizationComponent::ApplyCustomization()
{
    if (!CachedMeshComponent)
    {
        if (AActor* Owner = GetOwner())
        {
            CachedMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        }
    }

    if (CachedMeshComponent)
    {
        UpdateMeshAndMaterials();
        UpdateBodyMorphTargets();
    }
}

void UCharacterCustomizationComponent::SetBodyType(EChar_BodyType NewBodyType)
{
    CustomizationData.BodyType = NewBodyType;
    ApplyCustomization();
}

void UCharacterCustomizationComponent::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CustomizationData.SkinTone = NewSkinTone;
    ApplySkinMaterial();
}

void UCharacterCustomizationComponent::SetClothingSet(EChar_ClothingSet NewClothingSet)
{
    CustomizationData.ClothingSet = NewClothingSet;
    ApplyClothingMaterial();
}

void UCharacterCustomizationComponent::RandomizeAppearance()
{
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    CustomizationData.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 5);
    CustomizationData.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize clothing set
    int32 ClothingIndex = FMath::RandRange(0, 4);
    CustomizationData.ClothingSet = static_cast<EChar_ClothingSet>(ClothingIndex);
    
    // Randomize physical attributes
    CustomizationData.MuscleDefinition = FMath::FRandRange(0.2f, 0.9f);
    CustomizationData.BodyFat = FMath::FRandRange(0.1f, 0.6f);
    CustomizationData.ScarIntensity = FMath::FRandRange(0.0f, 0.8f);
    CustomizationData.WeatheringLevel = FMath::FRandRange(0.2f, 0.8f);
    
    ApplyCustomization();
}

void UCharacterCustomizationComponent::UpdateMeshAndMaterials()
{
    if (!CachedMeshComponent || !CustomizationAsset.IsValid())
    {
        return;
    }

    UChar_CustomizationAsset* Asset = CustomizationAsset.LoadSynchronous();
    if (!Asset)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterCustomizationComponent: CustomizationAsset is null"));
        return;
    }

    // Update mesh based on body type
    if (Asset->BodyMeshes.Contains(CustomizationData.BodyType))
    {
        TSoftObjectPtr<USkeletalMesh> MeshPtr = Asset->BodyMeshes[CustomizationData.BodyType];
        if (USkeletalMesh* NewMesh = MeshPtr.LoadSynchronous())
        {
            CachedMeshComponent->SetSkeletalMesh(NewMesh);
        }
    }

    // Apply materials
    ApplySkinMaterial();
    ApplyClothingMaterial();
}

void UCharacterCustomizationComponent::ApplySkinMaterial()
{
    if (!CachedMeshComponent || !CustomizationAsset.IsValid())
    {
        return;
    }

    UChar_CustomizationAsset* Asset = CustomizationAsset.LoadSynchronous();
    if (!Asset || !Asset->SkinMaterials.Contains(CustomizationData.SkinTone))
    {
        return;
    }

    TSoftObjectPtr<UMaterialInterface> MaterialPtr = Asset->SkinMaterials[CustomizationData.SkinTone];
    if (UMaterialInterface* SkinMaterial = MaterialPtr.LoadSynchronous())
    {
        // Create dynamic material instance for runtime parameter adjustment
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(SkinMaterial, this);
        if (DynamicMaterial)
        {
            // Apply weathering and scar parameters
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CustomizationData.WeatheringLevel);
            DynamicMaterial->SetScalarParameterValue(TEXT("ScarIntensity"), CustomizationData.ScarIntensity);
            DynamicMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), CustomizationData.MuscleDefinition);
            
            // Apply to skin material slots (typically 0 for body)
            CachedMeshComponent->SetMaterial(0, DynamicMaterial);
        }
    }
}

void UCharacterCustomizationComponent::ApplyClothingMaterial()
{
    if (!CachedMeshComponent || !CustomizationAsset.IsValid())
    {
        return;
    }

    UChar_CustomizationAsset* Asset = CustomizationAsset.LoadSynchronous();
    if (!Asset || !Asset->ClothingMaterials.Contains(CustomizationData.ClothingSet))
    {
        return;
    }

    TSoftObjectPtr<UMaterialInterface> MaterialPtr = Asset->ClothingMaterials[CustomizationData.ClothingSet];
    if (UMaterialInterface* ClothingMaterial = MaterialPtr.LoadSynchronous())
    {
        // Create dynamic material instance
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ClothingMaterial, this);
        if (DynamicMaterial)
        {
            // Apply weathering to clothing as well
            DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CustomizationData.WeatheringLevel);
            
            // Apply to clothing material slots (typically 1+ for clothing pieces)
            for (int32 MaterialIndex = 1; MaterialIndex < CachedMeshComponent->GetNumMaterials(); ++MaterialIndex)
            {
                CachedMeshComponent->SetMaterial(MaterialIndex, DynamicMaterial);
            }
        }
    }
}

void UCharacterCustomizationComponent::UpdateBodyMorphTargets()
{
    if (!CachedMeshComponent)
    {
        return;
    }

    // Apply morph targets based on body type and physical attributes
    switch (CustomizationData.BodyType)
    {
        case EChar_BodyType::Lean:
            CachedMeshComponent->SetMorphTarget(TEXT("BodyFat"), FMath::Clamp(CustomizationData.BodyFat * 0.5f, 0.0f, 0.3f));
            CachedMeshComponent->SetMorphTarget(TEXT("Muscle"), FMath::Clamp(CustomizationData.MuscleDefinition * 0.6f, 0.0f, 0.6f));
            break;
            
        case EChar_BodyType::Athletic:
            CachedMeshComponent->SetMorphTarget(TEXT("BodyFat"), FMath::Clamp(CustomizationData.BodyFat * 0.7f, 0.0f, 0.4f));
            CachedMeshComponent->SetMorphTarget(TEXT("Muscle"), FMath::Clamp(CustomizationData.MuscleDefinition * 0.8f, 0.0f, 0.8f));
            break;
            
        case EChar_BodyType::Muscular:
            CachedMeshComponent->SetMorphTarget(TEXT("BodyFat"), FMath::Clamp(CustomizationData.BodyFat * 0.6f, 0.0f, 0.4f));
            CachedMeshComponent->SetMorphTarget(TEXT("Muscle"), FMath::Clamp(CustomizationData.MuscleDefinition, 0.6f, 1.0f));
            break;
            
        case EChar_BodyType::Heavy:
            CachedMeshComponent->SetMorphTarget(TEXT("BodyFat"), FMath::Clamp(CustomizationData.BodyFat, 0.4f, 1.0f));
            CachedMeshComponent->SetMorphTarget(TEXT("Muscle"), FMath::Clamp(CustomizationData.MuscleDefinition * 0.7f, 0.0f, 0.7f));
            break;
    }

    // Apply additional morph targets for tribal characteristics
    CachedMeshComponent->SetMorphTarget(TEXT("Weathering"), CustomizationData.WeatheringLevel);
    CachedMeshComponent->SetMorphTarget(TEXT("BattleScars"), CustomizationData.ScarIntensity);
}