#include "Char_MetaHumanController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"

UChar_MetaHumanController::UChar_MetaHumanController()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default appearance settings
    AppearanceSettings = FChar_AppearanceSettings();
}

void UChar_MetaHumanController::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        TargetMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (TargetMeshComponent)
        {
            // Apply initial appearance settings
            ApplyAppearanceSettings();
        }
    }
}

void UChar_MetaHumanController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_MetaHumanController::ApplyAppearanceSettings()
{
    if (!TargetMeshComponent)
    {
        return;
    }

    // Update materials based on appearance settings
    UpdateSkinMaterial();
    UpdateHairMaterial();
    UpdateEyeMaterial();
    UpdateMeshMorphTargets();
}

void UChar_MetaHumanController::SetSkinTone(EChar_SkinTone NewSkinTone)
{
    AppearanceSettings.SkinTone = NewSkinTone;
    UpdateSkinMaterial();
}

void UChar_MetaHumanController::SetBodyBuild(EChar_BodyBuild NewBodyBuild)
{
    AppearanceSettings.BodyBuild = NewBodyBuild;
    UpdateMeshMorphTargets();
}

void UChar_MetaHumanController::SetWeatheringLevel(float NewWeatheringLevel)
{
    AppearanceSettings.WeatheringLevel = FMath::Clamp(NewWeatheringLevel, 0.0f, 1.0f);
    UpdateSkinMaterial();
}

void UChar_MetaHumanController::SetScarLevel(float NewScarLevel)
{
    AppearanceSettings.ScarLevel = FMath::Clamp(NewScarLevel, 0.0f, 1.0f);
    UpdateSkinMaterial();
}

void UChar_MetaHumanController::SetHairColor(FLinearColor NewHairColor)
{
    AppearanceSettings.HairColor = NewHairColor;
    UpdateHairMaterial();
}

void UChar_MetaHumanController::SetEyeColor(FLinearColor NewEyeColor)
{
    AppearanceSettings.EyeColor = NewEyeColor;
    UpdateEyeMaterial();
}

void UChar_MetaHumanController::RandomizeAppearance()
{
    // Randomize skin tone
    int32 SkinToneIndex = FMath::RandRange(0, 4);
    AppearanceSettings.SkinTone = static_cast<EChar_SkinTone>(SkinToneIndex);
    
    // Randomize body build
    int32 BodyBuildIndex = FMath::RandRange(0, 3);
    AppearanceSettings.BodyBuild = static_cast<EChar_BodyBuild>(BodyBuildIndex);
    
    // Randomize weathering and scars
    AppearanceSettings.WeatheringLevel = FMath::RandRange(0.2f, 0.8f);
    AppearanceSettings.ScarLevel = FMath::RandRange(0.0f, 0.6f);
    
    // Randomize hair color (earth tones)
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.2f, 0.1f, 0.05f, 1.0f), // Dark Brown
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f),  // Brown
        FLinearColor(0.6f, 0.4f, 0.2f, 1.0f),  // Light Brown
        FLinearColor(0.8f, 0.6f, 0.3f, 1.0f)   // Blonde
    };
    AppearanceSettings.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 1)];
    
    // Randomize eye color
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.4f, 0.2f, 0.1f, 1.0f), // Brown
        FLinearColor(0.2f, 0.4f, 0.6f, 1.0f), // Blue
        FLinearColor(0.3f, 0.5f, 0.2f, 1.0f), // Green
        FLinearColor(0.5f, 0.4f, 0.2f, 1.0f)  // Hazel
    };
    AppearanceSettings.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Apply all changes
    ApplyAppearanceSettings();
}

void UChar_MetaHumanController::LoadPresetAppearance(const FString& PresetName)
{
    // TODO: Implement preset loading from save system
    UE_LOG(LogTemp, Warning, TEXT("LoadPresetAppearance: %s - Not yet implemented"), *PresetName);
}

void UChar_MetaHumanController::SaveAppearancePreset(const FString& PresetName)
{
    // TODO: Implement preset saving to save system
    UE_LOG(LogTemp, Warning, TEXT("SaveAppearancePreset: %s - Not yet implemented"), *PresetName);
}

void UChar_MetaHumanController::UpdateSkinMaterial()
{
    if (!TargetMeshComponent || !SkinMaterial)
    {
        return;
    }

    // Create dynamic material instance if needed
    if (!DynamicSkinMaterial)
    {
        DynamicSkinMaterial = UMaterialInstanceDynamic::Create(SkinMaterial, this);
        if (DynamicSkinMaterial)
        {
            TargetMeshComponent->SetMaterial(0, DynamicSkinMaterial);
        }
    }

    if (DynamicSkinMaterial)
    {
        // Set skin tone parameter
        float SkinToneValue = 0.5f;
        switch (AppearanceSettings.SkinTone)
        {
            case EChar_SkinTone::Fair:
                SkinToneValue = 0.9f;
                break;
            case EChar_SkinTone::Medium:
                SkinToneValue = 0.7f;
                break;
            case EChar_SkinTone::Olive:
                SkinToneValue = 0.5f;
                break;
            case EChar_SkinTone::Dark:
                SkinToneValue = 0.3f;
                break;
            case EChar_SkinTone::VeryDark:
                SkinToneValue = 0.1f;
                break;
        }
        
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("SkinTone"), SkinToneValue);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("WeatheringLevel"), AppearanceSettings.WeatheringLevel);
        DynamicSkinMaterial->SetScalarParameterValue(TEXT("ScarLevel"), AppearanceSettings.ScarLevel);
    }
}

void UChar_MetaHumanController::UpdateHairMaterial()
{
    if (!TargetMeshComponent || !HairMaterial)
    {
        return;
    }

    // Create dynamic material instance if needed
    if (!DynamicHairMaterial)
    {
        DynamicHairMaterial = UMaterialInstanceDynamic::Create(HairMaterial, this);
        if (DynamicHairMaterial)
        {
            TargetMeshComponent->SetMaterial(1, DynamicHairMaterial);
        }
    }

    if (DynamicHairMaterial)
    {
        DynamicHairMaterial->SetVectorParameterValue(TEXT("HairColor"), AppearanceSettings.HairColor);
    }
}

void UChar_MetaHumanController::UpdateEyeMaterial()
{
    if (!TargetMeshComponent || !EyeMaterial)
    {
        return;
    }

    // Create dynamic material instance if needed
    if (!DynamicEyeMaterial)
    {
        DynamicEyeMaterial = UMaterialInstanceDynamic::Create(EyeMaterial, this);
        if (DynamicEyeMaterial)
        {
            TargetMeshComponent->SetMaterial(2, DynamicEyeMaterial);
        }
    }

    if (DynamicEyeMaterial)
    {
        DynamicEyeMaterial->SetVectorParameterValue(TEXT("EyeColor"), AppearanceSettings.EyeColor);
    }
}

void UChar_MetaHumanController::UpdateMeshMorphTargets()
{
    if (!TargetMeshComponent)
    {
        return;
    }

    // Apply body build morph targets
    float BuildValue = 0.0f;
    switch (AppearanceSettings.BodyBuild)
    {
        case EChar_BodyBuild::Lean:
            BuildValue = -0.5f;
            break;
        case EChar_BodyBuild::Athletic:
            BuildValue = 0.0f;
            break;
        case EChar_BodyBuild::Stocky:
            BuildValue = 0.3f;
            break;
        case EChar_BodyBuild::Muscular:
            BuildValue = 0.7f;
            break;
    }

    // Set morph target values (these would be actual morph target names in a real MetaHuman)
    TargetMeshComponent->SetMorphTarget(TEXT("BodyBuild"), BuildValue);
    TargetMeshComponent->SetMorphTarget(TEXT("Muscle"), BuildValue * 0.8f);
}