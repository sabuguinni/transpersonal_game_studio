#include "CharacterAppearanceSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"

UCharacterAppearanceSystem::UCharacterAppearanceSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    CurrentAppearance = FChar_AppearanceData();
}

void UCharacterAppearanceSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Cache the skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        CachedMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    // Apply the current appearance
    ApplyAppearance(CurrentAppearance);
}

void UCharacterAppearanceSystem::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    
    UpdateMeshScale();
    UpdateSkinMaterial();
    UpdateClothingMaterial();
    
    UE_LOG(LogTemp, Log, TEXT("Applied character appearance: BodyType=%d, SkinTone=%d, ClothingSet=%d"), 
           (int32)CurrentAppearance.BodyType, 
           (int32)CurrentAppearance.SkinTone, 
           (int32)CurrentAppearance.ClothingSet);
}

void UCharacterAppearanceSystem::SetBodyType(EChar_BodyType NewBodyType)
{
    CurrentAppearance.BodyType = NewBodyType;
    UpdateMeshScale();
}

void UCharacterAppearanceSystem::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    UpdateSkinMaterial();
}

void UCharacterAppearanceSystem::SetClothingSet(EChar_ClothingSet NewClothingSet)
{
    CurrentAppearance.ClothingSet = NewClothingSet;
    UpdateClothingMaterial();
}

void UCharacterAppearanceSystem::RandomizeAppearance()
{
    // Randomize body type
    int32 BodyTypeCount = (int32)EChar_BodyType::Heavy + 1;
    CurrentAppearance.BodyType = (EChar_BodyType)FMath::RandRange(0, BodyTypeCount - 1);
    
    // Randomize skin tone
    int32 SkinToneCount = (int32)EChar_SkinTone::Deep + 1;
    CurrentAppearance.SkinTone = (EChar_SkinTone)FMath::RandRange(0, SkinToneCount - 1);
    
    // Randomize clothing set
    int32 ClothingCount = (int32)EChar_ClothingSet::Warrior + 1;
    CurrentAppearance.ClothingSet = (EChar_ClothingSet)FMath::RandRange(0, ClothingCount - 1);
    
    // Randomize body scale slightly
    float ScaleVariation = 0.15f;
    CurrentAppearance.BodyScale = FVector(
        FMath::RandRange(1.0f - ScaleVariation, 1.0f + ScaleVariation),
        FMath::RandRange(1.0f - ScaleVariation, 1.0f + ScaleVariation),
        FMath::RandRange(1.0f - ScaleVariation, 1.0f + ScaleVariation)
    );
    
    // Set skin color based on skin tone
    switch (CurrentAppearance.SkinTone)
    {
        case EChar_SkinTone::Pale:
            CurrentAppearance.SkinColor = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
            break;
        case EChar_SkinTone::Fair:
            CurrentAppearance.SkinColor = FLinearColor(0.9f, 0.75f, 0.65f, 1.0f);
            break;
        case EChar_SkinTone::Medium:
            CurrentAppearance.SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
            break;
        case EChar_SkinTone::Tan:
            CurrentAppearance.SkinColor = FLinearColor(0.7f, 0.5f, 0.35f, 1.0f);
            break;
        case EChar_SkinTone::Dark:
            CurrentAppearance.SkinColor = FLinearColor(0.5f, 0.35f, 0.25f, 1.0f);
            break;
        case EChar_SkinTone::Deep:
            CurrentAppearance.SkinColor = FLinearColor(0.3f, 0.2f, 0.15f, 1.0f);
            break;
    }
    
    ApplyAppearance(CurrentAppearance);
}

void UCharacterAppearanceSystem::UpdateMeshScale()
{
    if (!CachedMeshComponent)
        return;
    
    FVector FinalScale = CurrentAppearance.BodyScale;
    
    // Adjust scale based on body type
    switch (CurrentAppearance.BodyType)
    {
        case EChar_BodyType::Lean:
            FinalScale *= FVector(0.9f, 0.9f, 1.0f);
            break;
        case EChar_BodyType::Athletic:
            FinalScale *= FVector(1.0f, 1.0f, 1.0f);
            break;
        case EChar_BodyType::Muscular:
            FinalScale *= FVector(1.1f, 1.1f, 1.05f);
            break;
        case EChar_BodyType::Heavy:
            FinalScale *= FVector(1.2f, 1.2f, 1.0f);
            break;
    }
    
    CachedMeshComponent->SetWorldScale3D(FinalScale);
}

void UCharacterAppearanceSystem::UpdateSkinMaterial()
{
    if (!CachedMeshComponent)
        return;
    
    // Create dynamic material instance for skin
    if (SkinMaterials.Num() > 0 && SkinMaterials[0])
    {
        UMaterialInstanceDynamic* DynamicSkinMaterial = UMaterialInstanceDynamic::Create(SkinMaterials[0], this);
        if (DynamicSkinMaterial)
        {
            DynamicSkinMaterial->SetVectorParameterValue(TEXT("SkinColor"), CurrentAppearance.SkinColor);
            CachedMeshComponent->SetMaterial(0, DynamicSkinMaterial);
        }
    }
}

void UCharacterAppearanceSystem::UpdateClothingMaterial()
{
    if (!CachedMeshComponent)
        return;
    
    // Apply clothing material based on clothing set
    int32 ClothingIndex = (int32)CurrentAppearance.ClothingSet;
    if (ClothingMaterials.IsValidIndex(ClothingIndex) && ClothingMaterials[ClothingIndex])
    {
        CachedMeshComponent->SetMaterial(1, ClothingMaterials[ClothingIndex]);
    }
}