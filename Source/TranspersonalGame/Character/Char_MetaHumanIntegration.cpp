#include "Char_MetaHumanIntegration.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

UChar_MetaHumanIntegration::UChar_MetaHumanIntegration()
{
    PrimaryComponentTick.bCanEverTick = true;
    TargetMeshComponent = nullptr;
}

void UChar_MetaHumanIntegration::BeginPlay()
{
    Super::BeginPlay();
    
    // Find the skeletal mesh component on the owner
    if (AActor* Owner = GetOwner())
    {
        TargetMeshComponent = Owner->FindComponentByClass<USkeletalMeshComponent>();
        if (TargetMeshComponent)
        {
            UE_LOG(LogTemp, Log, TEXT("MetaHuman Integration: Found target mesh component"));
            InitializeDiversityPresets();
            
            // Apply default configuration
            if (MetaHumanConfig.PresetType != EChar_MetaHumanPreset::Custom)
            {
                ApplyMetaHumanPreset(MetaHumanConfig.PresetType);
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("MetaHuman Integration: No skeletal mesh component found on owner"));
        }
    }
}

void UChar_MetaHumanIntegration::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UChar_MetaHumanIntegration::ApplyMetaHumanPreset(EChar_MetaHumanPreset PresetType)
{
    FChar_MetaHumanConfig PresetConfig = GetPresetConfiguration(PresetType);
    ApplyCustomConfiguration(PresetConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Applied MetaHuman preset: %d"), (int32)PresetType);
}

void UChar_MetaHumanIntegration::ApplyCustomConfiguration(const FChar_MetaHumanConfig& Config)
{
    if (!TargetMeshComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot apply configuration - no target mesh component"));
        return;
    }

    MetaHumanConfig = Config;
    
    // Apply base mesh if specified
    if (Config.BaseMesh.IsValid())
    {
        if (USkeletalMesh* LoadedMesh = Config.BaseMesh.LoadSynchronous())
        {
            TargetMeshComponent->SetSkeletalMesh(LoadedMesh);
            UE_LOG(LogTemp, Log, TEXT("Applied base mesh: %s"), *LoadedMesh->GetName());
        }
    }
    
    // Update material parameters
    UpdateMaterialParameters();
}

void UChar_MetaHumanIntegration::GenerateRandomizedAppearance()
{
    FChar_MetaHumanConfig RandomConfig;
    
    // Randomize age and weathering
    RandomConfig.AgeModifier = FMath::RandRange(0.2f, 0.8f);
    RandomConfig.WeatheringIntensity = FMath::RandRange(0.1f, 0.6f);
    
    // Randomize skin tone
    float HueVariation = FMath::RandRange(0.05f, 0.15f);
    float SatVariation = FMath::RandRange(0.4f, 0.8f);
    float ValVariation = FMath::RandRange(0.3f, 0.7f);
    RandomConfig.SkinTone = FLinearColor(ValVariation + HueVariation, ValVariation, ValVariation - HueVariation * 0.5f, 1.0f);
    
    // Select random preset as base
    int32 RandomPreset = FMath::RandRange(0, 4);
    RandomConfig.PresetType = (EChar_MetaHumanPreset)RandomPreset;
    
    ApplyCustomConfiguration(RandomConfig);
    UE_LOG(LogTemp, Log, TEXT("Generated randomized appearance"));
}

void UChar_MetaHumanIntegration::SetAgeAndWeathering(float Age, float Weathering)
{
    MetaHumanConfig.AgeModifier = FMath::Clamp(Age, 0.0f, 1.0f);
    MetaHumanConfig.WeatheringIntensity = FMath::Clamp(Weathering, 0.0f, 1.0f);
    
    UpdateMaterialParameters();
    UE_LOG(LogTemp, Log, TEXT("Updated age: %f, weathering: %f"), Age, Weathering);
}

void UChar_MetaHumanIntegration::UpdateMaterialParameters()
{
    if (!TargetMeshComponent)
        return;

    // Update face materials
    for (int32 i = 0; i < MetaHumanConfig.FaceMaterials.Num(); i++)
    {
        if (UMaterialInterface* FaceMaterial = MetaHumanConfig.FaceMaterials[i].LoadSynchronous())
        {
            if (UMaterialInstanceDynamic* DynMaterial = TargetMeshComponent->CreateDynamicMaterialInstance(i, FaceMaterial))
            {
                ApplyMaterialParameters(DynMaterial, MetaHumanConfig);
            }
        }
    }
    
    // Update body materials
    for (int32 i = 0; i < MetaHumanConfig.BodyMaterials.Num(); i++)
    {
        if (UMaterialInterface* BodyMaterial = MetaHumanConfig.BodyMaterials[i].LoadSynchronous())
        {
            int32 MaterialIndex = MetaHumanConfig.FaceMaterials.Num() + i;
            if (UMaterialInstanceDynamic* DynMaterial = TargetMeshComponent->CreateDynamicMaterialInstance(MaterialIndex, BodyMaterial))
            {
                ApplyMaterialParameters(DynMaterial, MetaHumanConfig);
            }
        }
    }
}

void UChar_MetaHumanIntegration::InitializeDiversityPresets()
{
    DiversityPresets.Empty();
    
    // Tribal Elder preset
    FChar_MetaHumanConfig ElderConfig;
    ElderConfig.PresetType = EChar_MetaHumanPreset::TribalElder;
    ElderConfig.AgeModifier = 0.8f;
    ElderConfig.WeatheringIntensity = 0.7f;
    ElderConfig.SkinTone = FLinearColor(0.6f, 0.4f, 0.3f, 1.0f);
    DiversityPresets.Add(ElderConfig);
    
    // Hunter preset
    FChar_MetaHumanConfig HunterConfig;
    HunterConfig.PresetType = EChar_MetaHumanPreset::Hunter;
    HunterConfig.AgeModifier = 0.4f;
    HunterConfig.WeatheringIntensity = 0.5f;
    HunterConfig.SkinTone = FLinearColor(0.7f, 0.5f, 0.4f, 1.0f);
    DiversityPresets.Add(HunterConfig);
    
    // Gatherer preset
    FChar_MetaHumanConfig GathererConfig;
    GathererConfig.PresetType = EChar_MetaHumanPreset::Gatherer;
    GathererConfig.AgeModifier = 0.2f;
    GathererConfig.WeatheringIntensity = 0.2f;
    GathererConfig.SkinTone = FLinearColor(0.8f, 0.6f, 0.5f, 1.0f);
    DiversityPresets.Add(GathererConfig);
    
    // Crafter preset
    FChar_MetaHumanConfig CrafterConfig;
    CrafterConfig.PresetType = EChar_MetaHumanPreset::Crafter;
    CrafterConfig.AgeModifier = 0.5f;
    CrafterConfig.WeatheringIntensity = 0.4f;
    CrafterConfig.SkinTone = FLinearColor(0.75f, 0.55f, 0.45f, 1.0f);
    DiversityPresets.Add(CrafterConfig);
    
    // Scout preset
    FChar_MetaHumanConfig ScoutConfig;
    ScoutConfig.PresetType = EChar_MetaHumanPreset::Scout;
    ScoutConfig.AgeModifier = 0.3f;
    ScoutConfig.WeatheringIntensity = 0.6f;
    ScoutConfig.SkinTone = FLinearColor(0.65f, 0.45f, 0.35f, 1.0f);
    DiversityPresets.Add(ScoutConfig);
    
    UE_LOG(LogTemp, Log, TEXT("Initialized %d diversity presets"), DiversityPresets.Num());
}

void UChar_MetaHumanIntegration::ApplyMaterialParameters(UMaterialInterface* Material, const FChar_MetaHumanConfig& Config)
{
    if (UMaterialInstanceDynamic* DynMaterial = Cast<UMaterialInstanceDynamic>(Material))
    {
        // Apply skin tone
        DynMaterial->SetVectorParameterValue(TEXT("SkinTone"), Config.SkinTone);
        
        // Apply age modifier
        DynMaterial->SetScalarParameterValue(TEXT("AgeModifier"), Config.AgeModifier);
        
        // Apply weathering
        DynMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), Config.WeatheringIntensity);
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Applied material parameters to dynamic material"));
    }
}

FChar_MetaHumanConfig UChar_MetaHumanIntegration::GetPresetConfiguration(EChar_MetaHumanPreset PresetType)
{
    for (const FChar_MetaHumanConfig& Preset : DiversityPresets)
    {
        if (Preset.PresetType == PresetType)
        {
            return Preset;
        }
    }
    
    // Return default if not found
    FChar_MetaHumanConfig DefaultConfig;
    DefaultConfig.PresetType = PresetType;
    return DefaultConfig;
}