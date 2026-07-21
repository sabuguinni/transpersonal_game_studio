#include "Char_PlayerAppearanceManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"

UChar_PlayerAppearanceManager::UChar_PlayerAppearanceManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    CharacterMesh = nullptr;
}

void UChar_PlayerAppearanceManager::BeginPlay()
{
    Super::BeginPlay();

    // Find the character's skeletal mesh component
    if (AActor* Owner = GetOwner())
    {
        CharacterMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (CharacterMesh)
        {
            UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Found character mesh component"));
            ApplyAppearance(CurrentAppearance);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerAppearanceManager: No skeletal mesh component found on owner"));
        }
    }
}

void UChar_PlayerAppearanceManager::ApplyAppearance(const FChar_AppearanceData& NewAppearance)
{
    CurrentAppearance = NewAppearance;

    if (!CharacterMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("PlayerAppearanceManager: No character mesh available"));
        return;
    }

    UpdateCharacterMesh();
    UpdateSkinMaterial();
    UpdateClothing();

    UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Applied new appearance - SkinTone: %d, BodyBuild: %d"), 
           (int32)NewAppearance.SkinTone, (int32)NewAppearance.BodyBuild);
}

void UChar_PlayerAppearanceManager::RandomizeAppearance()
{
    FChar_AppearanceData RandomAppearance;

    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    RandomAppearance.SkinTone = (EChar_SkinTone)SkinToneIndex;

    // Randomize body build
    int32 BodyBuildIndex = FMath::RandRange(0, 4);
    RandomAppearance.BodyBuild = (EChar_BodyBuild)BodyBuildIndex;

    // Randomize physical attributes
    RandomAppearance.Height = FMath::RandRange(0.9f, 1.1f);
    RandomAppearance.Weight = FMath::RandRange(0.8f, 1.2f);

    // Randomize hair color (earth tones)
    float HairHue = FMath::RandRange(0.0f, 0.1f); // Brown to black range
    RandomAppearance.HairColor = FLinearColor::MakeFromHSV8(HairHue * 255, 200, FMath::RandRange(50, 150));

    // Randomize eye color (natural colors)
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.3f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.2f, 1.0f), // Green
        FLinearColor(0.2f, 0.3f, 0.6f, 1.0f), // Blue
        FLinearColor(0.4f, 0.4f, 0.4f, 1.0f)  // Gray
    };
    RandomAppearance.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];

    ApplyAppearance(RandomAppearance);
}

void UChar_PlayerAppearanceManager::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    UpdateSkinMaterial();
}

void UChar_PlayerAppearanceManager::SetBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    CurrentAppearance.BodyBuild = NewBodyBuild;
    UpdateCharacterMesh();
}

void UChar_PlayerAppearanceManager::SetClothing(USkeletalMesh* ClothingMesh, UMaterialInterface* ClothingMaterial)
{
    if (ClothingMesh)
    {
        CurrentAppearance.ClothingMesh = ClothingMesh;
    }
    
    if (ClothingMaterial)
    {
        CurrentAppearance.ClothingMaterial = ClothingMaterial;
    }

    UpdateClothing();
}

void UChar_PlayerAppearanceManager::UpdateCharacterMesh()
{
    if (!CharacterMesh)
        return;

    // Apply body build scaling
    FVector Scale = FVector(1.0f);
    switch (CurrentAppearance.BodyBuild)
    {
        case EChar_BodyBuild::Lean:
            Scale = FVector(0.95f, 0.95f, 1.0f);
            break;
        case EChar_BodyBuild::Athletic:
            Scale = FVector(1.0f, 1.0f, 1.0f);
            break;
        case EChar_BodyBuild::Muscular:
            Scale = FVector(1.1f, 1.1f, 1.0f);
            break;
        case EChar_BodyBuild::Stocky:
            Scale = FVector(1.15f, 1.15f, 0.95f);
            break;
        case EChar_BodyBuild::Tall:
            Scale = FVector(0.95f, 0.95f, 1.1f);
            break;
    }

    // Apply height and weight modifiers
    Scale.X *= CurrentAppearance.Weight;
    Scale.Y *= CurrentAppearance.Weight;
    Scale.Z *= CurrentAppearance.Height;

    CharacterMesh->SetWorldScale3D(Scale);

    UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Updated character mesh scale to %s"), *Scale.ToString());
}

void UChar_PlayerAppearanceManager::UpdateSkinMaterial()
{
    if (!CharacterMesh)
        return;

    // Create dynamic material instance for skin
    UMaterialInterface* BaseMaterial = CharacterMesh->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            // Set skin tone color based on enum
            FLinearColor SkinColor;
            switch (CurrentAppearance.SkinTone)
            {
                case EChar_SkinTone::Fair:
                    SkinColor = FLinearColor(0.95f, 0.85f, 0.75f, 1.0f);
                    break;
                case EChar_SkinTone::Medium:
                    SkinColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
                    break;
                case EChar_SkinTone::Tan:
                    SkinColor = FLinearColor(0.7f, 0.6f, 0.45f, 1.0f);
                    break;
                case EChar_SkinTone::Dark:
                    SkinColor = FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
                    break;
                case EChar_SkinTone::Weathered:
                    SkinColor = FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
                    break;
            }

            DynamicMaterial->SetVectorParameterValue(FName("SkinColor"), SkinColor);
            DynamicMaterial->SetVectorParameterValue(FName("HairColor"), CurrentAppearance.HairColor);
            DynamicMaterial->SetVectorParameterValue(FName("EyeColor"), CurrentAppearance.EyeColor);

            CharacterMesh->SetMaterial(0, DynamicMaterial);

            UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Updated skin material with color %s"), *SkinColor.ToString());
        }
    }
}

void UChar_PlayerAppearanceManager::UpdateClothing()
{
    if (!CharacterMesh)
        return;

    // Apply clothing mesh if available
    if (CurrentAppearance.ClothingMesh.IsValid())
    {
        USkeletalMesh* ClothingMesh = CurrentAppearance.ClothingMesh.LoadSynchronous();
        if (ClothingMesh)
        {
            // In a full implementation, this would handle clothing as separate mesh components
            // For now, we'll just log the clothing change
            UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Clothing mesh updated"));
        }
    }

    // Apply clothing material if available
    if (CurrentAppearance.ClothingMaterial.IsValid())
    {
        UMaterialInterface* ClothingMat = CurrentAppearance.ClothingMaterial.LoadSynchronous();
        if (ClothingMat)
        {
            // Apply to clothing material slots (typically slots 1+)
            for (int32 i = 1; i < CharacterMesh->GetNumMaterials(); i++)
            {
                CharacterMesh->SetMaterial(i, ClothingMat);
            }
            UE_LOG(LogTemp, Log, TEXT("PlayerAppearanceManager: Clothing material updated"));
        }
    }
}