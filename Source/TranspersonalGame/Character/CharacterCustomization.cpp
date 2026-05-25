#include "CharacterCustomization.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/Engine.h"

UChar_CustomizationComponent::UChar_CustomizationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance
    AppearanceData.SkinTone = EChar_SkinTone::Medium;
    AppearanceData.BodyType = EChar_BodyType::Athletic;
    AppearanceData.ClothingStyle = EChar_ClothingStyle::Hunter;
    AppearanceData.HairColor = FLinearColor::Black;
    AppearanceData.EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    AppearanceData.Height = 1.0f;
    AppearanceData.Weight = 1.0f;
    AppearanceData.bHasTribalMarkings = false;
    AppearanceData.bHasBattleScars = false;
}

void UChar_CustomizationComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the character mesh component
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            UpdateCharacterMesh();
        }
    }
}

void UChar_CustomizationComponent::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    AppearanceData = NewAppearance;
    UpdateCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Applied new appearance to character"));
}

void UChar_CustomizationComponent::RandomizeAppearance()
{
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 3);
    AppearanceData.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize body type
    int32 BodyTypeIndex = FMath::RandRange(0, 3);
    AppearanceData.BodyType = static_cast<EChar_BodyType>(BodyTypeIndex);
    
    // Randomize clothing style
    int32 ClothingIndex = FMath::RandRange(0, 3);
    AppearanceData.ClothingStyle = static_cast<EChar_ClothingStyle>(ClothingIndex);
    
    // Randomize colors
    AppearanceData.HairColor = FLinearColor(
        FMath::RandRange(0.1f, 0.9f),
        FMath::RandRange(0.1f, 0.6f),
        FMath::RandRange(0.1f, 0.4f),
        1.0f
    );
    
    AppearanceData.EyeColor = FLinearColor(
        FMath::RandRange(0.2f, 0.8f),
        FMath::RandRange(0.2f, 0.8f),
        FMath::RandRange(0.2f, 0.8f),
        1.0f
    );
    
    // Randomize physical attributes
    AppearanceData.Height = FMath::RandRange(0.9f, 1.1f);
    AppearanceData.Weight = FMath::RandRange(0.8f, 1.2f);
    
    // Randomize markings and scars
    AppearanceData.bHasTribalMarkings = FMath::RandBool();
    AppearanceData.bHasBattleScars = FMath::RandBool();
    
    UpdateCharacterMesh();
    
    UE_LOG(LogTemp, Log, TEXT("Randomized character appearance"));
}

void UChar_CustomizationComponent::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    AppearanceData.SkinTone = NewSkinTone;
    UpdateMaterials();
}

void UChar_CustomizationComponent::SetBodyType(EChar_BodyType NewBodyType)
{
    AppearanceData.BodyType = NewBodyType;
    UpdateCharacterMesh();
}

void UChar_CustomizationComponent::SetClothingStyle(EChar_ClothingStyle NewStyle)
{
    AppearanceData.ClothingStyle = NewStyle;
    UpdateClothing();
}

void UChar_CustomizationComponent::UpdateCharacterMesh()
{
    if (!CharacterMesh)
        return;
    
    // Apply scale based on height and weight
    FVector NewScale = FVector(
        AppearanceData.Weight,
        AppearanceData.Weight,
        AppearanceData.Height
    );
    CharacterMesh->SetRelativeScale3D(NewScale);
    
    UpdateMaterials();
    UpdateClothing();
}

void UChar_CustomizationComponent::UpdateMaterials()
{
    if (!CharacterMesh || SkinMaterials.Num() == 0)
        return;
    
    // Apply skin material based on skin tone
    int32 MaterialIndex = static_cast<int32>(AppearanceData.SkinTone);
    if (SkinMaterials.IsValidIndex(MaterialIndex))
    {
        CharacterMesh->SetMaterial(0, SkinMaterials[MaterialIndex]);
    }
    
    // Create dynamic material instance for color customization
    if (UMaterialInterface* BaseMaterial = CharacterMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = CharacterMesh->CreateDynamicMaterialInstance(0, BaseMaterial);
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("HairColor"), AppearanceData.HairColor);
            DynamicMaterial->SetVectorParameterValue(TEXT("EyeColor"), AppearanceData.EyeColor);
            DynamicMaterial->SetScalarParameterValue(TEXT("HasTribalMarkings"), AppearanceData.bHasTribalMarkings ? 1.0f : 0.0f);
            DynamicMaterial->SetScalarParameterValue(TEXT("HasBattleScars"), AppearanceData.bHasBattleScars ? 1.0f : 0.0f);
        }
    }
}

void UChar_CustomizationComponent::UpdateClothing()
{
    if (!CharacterMesh)
        return;
    
    // Apply clothing mesh based on clothing style
    int32 ClothingIndex = static_cast<int32>(AppearanceData.ClothingStyle);
    if (ClothingMeshes.IsValidIndex(ClothingIndex) && ClothingMeshes[ClothingIndex])
    {
        CharacterMesh->SetSkeletalMesh(ClothingMeshes[ClothingIndex]);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Updated character clothing to style: %d"), ClothingIndex);
}