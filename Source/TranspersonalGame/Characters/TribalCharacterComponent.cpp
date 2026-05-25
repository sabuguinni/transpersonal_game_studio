#include "TribalCharacterComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

UTribalCharacterComponent::UTribalCharacterComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    CharacterMesh = nullptr;
    BaseMaterial = nullptr;
    DynamicMaterial = nullptr;
    
    // Initialize default appearance
    CurrentAppearance = FChar_TribalAppearance();
}

void UTribalCharacterComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the character mesh component if not set
    if (!CharacterMesh)
    {
        CharacterMesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
    }
    
    InitializeMaterial();
    ApplyAppearance(CurrentAppearance);
}

void UTribalCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UTribalCharacterComponent::ApplyAppearance(const FChar_TribalAppearance& NewAppearance)
{
    CurrentAppearance = NewAppearance;
    UpdateMaterialParameters();
    
    UE_LOG(LogTemp, Log, TEXT("Applied tribal appearance: SkinTone=%d, BodyBuild=%d, ClothingStyle=%d"), 
           (int32)NewAppearance.SkinTone, (int32)NewAppearance.BodyBuild, (int32)NewAppearance.ClothingStyle);
}

void UTribalCharacterComponent::RandomizeAppearance()
{
    FChar_TribalAppearance RandomAppearance;
    
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 3);
    RandomAppearance.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize body build
    int32 BodyBuildIndex = FMath::RandRange(0, 3);
    RandomAppearance.BodyBuild = static_cast<EChar_BodyBuild>(BodyBuildIndex);
    
    // Randomize clothing style
    int32 ClothingIndex = FMath::RandRange(0, 3);
    RandomAppearance.ClothingStyle = static_cast<EChar_ClothingStyle>(ClothingIndex);
    
    // Randomize features
    RandomAppearance.bHasTribalMarkings = FMath::RandBool();
    RandomAppearance.bHasBattleScars = FMath::RandRange(0, 100) < 30; // 30% chance
    RandomAppearance.bHasFeatherHeadband = FMath::RandBool();
    RandomAppearance.bHasBoneJewelry = FMath::RandRange(0, 100) < 70; // 70% chance
    
    // Randomize weathering
    RandomAppearance.DirtLevel = FMath::RandRange(0.1f, 0.8f);
    RandomAppearance.WeatheringLevel = FMath::RandRange(0.2f, 0.9f);
    
    ApplyAppearance(RandomAppearance);
}

void UTribalCharacterComponent::UpdateSkinTone(EChar_SkinTone NewSkinTone)
{
    CurrentAppearance.SkinTone = NewSkinTone;
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::UpdateBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    CurrentAppearance.BodyBuild = NewBodyBuild;
    
    if (CharacterMesh)
    {
        float Scale = GetBodyBuildScale(NewBodyBuild);
        FVector CurrentScale = CharacterMesh->GetComponentScale();
        CharacterMesh->SetWorldScale3D(FVector(CurrentScale.X * Scale, CurrentScale.Y * Scale, CurrentScale.Z));
    }
}

void UTribalCharacterComponent::UpdateClothingStyle(EChar_ClothingStyle NewClothingStyle)
{
    CurrentAppearance.ClothingStyle = NewClothingStyle;
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetTribalMarkings(bool bEnabled)
{
    CurrentAppearance.bHasTribalMarkings = bEnabled;
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetBattleScars(bool bEnabled)
{
    CurrentAppearance.bHasBattleScars = bEnabled;
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetDirtLevel(float NewDirtLevel)
{
    CurrentAppearance.DirtLevel = FMath::Clamp(NewDirtLevel, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::SetWeatheringLevel(float NewWeatheringLevel)
{
    CurrentAppearance.WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
    UpdateMaterialParameters();
}

void UTribalCharacterComponent::PreviewRandomAppearance()
{
    RandomizeAppearance();
    UE_LOG(LogTemp, Warning, TEXT("Previewing random tribal appearance in editor"));
}

void UTribalCharacterComponent::InitializeMaterial()
{
    if (CharacterMesh && BaseMaterial)
    {
        DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        if (DynamicMaterial)
        {
            CharacterMesh->SetMaterial(0, DynamicMaterial);
            UE_LOG(LogTemp, Log, TEXT("Initialized dynamic material for tribal character"));
        }
    }
}

void UTribalCharacterComponent::UpdateMaterialParameters()
{
    if (!DynamicMaterial)
    {
        return;
    }
    
    // Update skin tone
    FLinearColor SkinColor = GetSkinToneColor(CurrentAppearance.SkinTone);
    DynamicMaterial->SetVectorParameterValue(TEXT("SkinColor"), SkinColor);
    
    // Update tribal markings
    DynamicMaterial->SetScalarParameterValue(TEXT("TribalMarkings"), CurrentAppearance.bHasTribalMarkings ? 1.0f : 0.0f);
    
    // Update battle scars
    DynamicMaterial->SetScalarParameterValue(TEXT("BattleScars"), CurrentAppearance.bHasBattleScars ? 1.0f : 0.0f);
    
    // Update dirt and weathering
    DynamicMaterial->SetScalarParameterValue(TEXT("DirtLevel"), CurrentAppearance.DirtLevel);
    DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), CurrentAppearance.WeatheringLevel);
    
    // Update clothing style (affects material blend)
    float ClothingBlend = static_cast<float>(CurrentAppearance.ClothingStyle) / 3.0f;
    DynamicMaterial->SetScalarParameterValue(TEXT("ClothingStyle"), ClothingBlend);
    
    UE_LOG(LogTemp, Log, TEXT("Updated tribal character material parameters"));
}

FLinearColor UTribalCharacterComponent::GetSkinToneColor(EChar_SkinTone SkinTone)
{
    switch (SkinTone)
    {
        case EChar_SkinTone::Light:
            return FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
        case EChar_SkinTone::Medium:
            return FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
        case EChar_SkinTone::Dark:
            return FLinearColor(0.5f, 0.4f, 0.3f, 1.0f);
        case EChar_SkinTone::Weathered:
            return FLinearColor(0.6f, 0.5f, 0.4f, 1.0f);
        default:
            return FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
    }
}

float UTribalCharacterComponent::GetBodyBuildScale(EChar_BodyBuild BodyBuild)
{
    switch (BodyBuild)
    {
        case EChar_BodyBuild::Lean:
            return 0.9f;
        case EChar_BodyBuild::Athletic:
            return 1.0f;
        case EChar_BodyBuild::Muscular:
            return 1.1f;
        case EChar_BodyBuild::Stocky:
            return 1.05f;
        default:
            return 1.0f;
    }
}