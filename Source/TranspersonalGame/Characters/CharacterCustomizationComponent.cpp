#include "CharacterCustomizationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

UCharacterCustomizationComponent::UCharacterCustomizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    CharacterMesh = nullptr;
    DynamicSkinMaterial = nullptr;
    DynamicClothingMaterial = nullptr;
}

void UCharacterCustomizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Get the character mesh component
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            ApplyCustomization();
        }
    }
}

void UCharacterCustomizationComponent::ApplyCustomization()
{
    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("CharacterCustomizationComponent: No mesh component found"));
        return;
    }

    UpdateCharacterMesh();
    UpdateMaterials();
    UpdateBodyScale();
}

void UCharacterCustomizationComponent::SetBodyType(EChar_BodyType NewBodyType)
{
    CustomizationData.BodyType = NewBodyType;
    UpdateCharacterMesh();
    UpdateBodyScale();
}

void UCharacterCustomizationComponent::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CustomizationData.SkinTone = NewSkinTone;
    UpdateMaterials();
}

void UCharacterCustomizationComponent::SetClothingSet(EChar_ClothingSet NewClothingSet)
{
    CustomizationData.ClothingSet = NewClothingSet;
    UpdateMaterials();
}

void UCharacterCustomizationComponent::RandomizeAppearance()
{
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    CustomizationData.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);

    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 5);
    CustomizationData.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);

    // Randomize clothing
    int32 ClothingIndex = FMath::RandRange(0, 3);
    CustomizationData.ClothingSet = static_cast<EChar_ClothingSet>(ClothingIndex);

    // Randomize body scale and muscle definition
    CustomizationData.BodyScale = FMath::RandRange(0.9f, 1.1f);
    CustomizationData.MuscleDefinition = FMath::RandRange(0.8f, 1.2f);

    ApplyCustomization();
}

void UCharacterCustomizationComponent::UpdateCharacterMesh()
{
    if (!CharacterMesh || BodyTypeMeshes.Num() == 0)
    {
        return;
    }

    int32 MeshIndex = static_cast<int32>(CustomizationData.BodyType);
    if (BodyTypeMeshes.IsValidIndex(MeshIndex) && BodyTypeMeshes[MeshIndex])
    {
        CharacterMesh->SetSkeletalMesh(BodyTypeMeshes[MeshIndex]);
    }
}

void UCharacterCustomizationComponent::UpdateMaterials()
{
    if (!CharacterMesh)
    {
        return;
    }

    // Update skin tone material
    int32 SkinIndex = static_cast<int32>(CustomizationData.SkinTone);
    if (SkinToneMaterials.IsValidIndex(SkinIndex) && SkinToneMaterials[SkinIndex])
    {
        DynamicSkinMaterial = UMaterialInstanceDynamic::Create(SkinToneMaterials[SkinIndex], this);
        if (DynamicSkinMaterial)
        {
            // Apply muscle definition parameter
            DynamicSkinMaterial->SetScalarParameterValue(TEXT("MuscleDefinition"), CustomizationData.MuscleDefinition);
            CharacterMesh->SetMaterial(0, DynamicSkinMaterial);
        }
    }

    // Update clothing material
    int32 ClothingIndex = static_cast<int32>(CustomizationData.ClothingSet);
    if (ClothingMaterials.IsValidIndex(ClothingIndex) && ClothingMaterials[ClothingIndex])
    {
        DynamicClothingMaterial = UMaterialInstanceDynamic::Create(ClothingMaterials[ClothingIndex], this);
        if (DynamicClothingMaterial)
        {
            // Apply wear and weathering based on clothing type
            float WearLevel = 0.5f;
            switch (CustomizationData.ClothingSet)
            {
                case EChar_ClothingSet::Minimal:
                    WearLevel = 0.2f;
                    break;
                case EChar_ClothingSet::Hunter:
                    WearLevel = 0.7f;
                    break;
                case EChar_ClothingSet::Gatherer:
                    WearLevel = 0.4f;
                    break;
                case EChar_ClothingSet::Warrior:
                    WearLevel = 0.8f;
                    break;
            }
            DynamicClothingMaterial->SetScalarParameterValue(TEXT("WearLevel"), WearLevel);
            CharacterMesh->SetMaterial(1, DynamicClothingMaterial);
        }
    }
}

void UCharacterCustomizationComponent::UpdateBodyScale()
{
    if (!CharacterMesh)
    {
        return;
    }

    FVector NewScale = FVector(CustomizationData.BodyScale);
    
    // Adjust scale based on body type
    switch (CustomizationData.BodyType)
    {
        case EChar_BodyType::Lean:
            NewScale *= FVector(0.95f, 0.95f, 1.02f); // Thinner, slightly taller
            break;
        case EChar_BodyType::Athletic:
            NewScale *= FVector(1.0f, 1.0f, 1.0f); // Standard proportions
            break;
        case EChar_BodyType::Muscular:
            NewScale *= FVector(1.1f, 1.1f, 0.98f); // Broader, slightly shorter
            break;
        case EChar_BodyType::Heavy:
            NewScale *= FVector(1.15f, 1.15f, 0.95f); // Wider, shorter
            break;
    }

    CharacterMesh->SetWorldScale3D(NewScale);
}