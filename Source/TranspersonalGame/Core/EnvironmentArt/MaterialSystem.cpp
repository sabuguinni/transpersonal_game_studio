// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "MaterialSystem.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogMaterialSystem, Log, All);

// ============================================================================
// UMaterialConfigData Implementation
// ============================================================================

UMaterialConfigData::UMaterialConfigData()
{
    ConfigName = FText::FromString(TEXT("Default Material Config"));
    ConfigDescription = FText::FromString(TEXT("Default material configuration for prehistoric environments"));
    UpdateFrequency = 1.0f;
    bEnableRealTimeUpdates = true;
    
    // Initialize default parameters for prehistoric environment
    DefaultParameters.WetnessLevel = 0.2f;
    DefaultParameters.MoistureLevel = 0.6f;
    DefaultParameters.Temperature = 0.3f; // Warm prehistoric climate
    DefaultParameters.WindStrength = 0.4f;
    DefaultParameters.TimeOfDay = 0.5f;
    DefaultParameters.SeasonFactor = 0.7f; // Lush growth season
    DefaultParameters.WearFactor = 0.1f; // Minimal wear in pristine nature
    DefaultParameters.VegetationGrowth = 0.8f; // Rich vegetation
}

// ============================================================================
// UMaterialSystemComponent Implementation
// ============================================================================

UMaterialSystemComponent::UMaterialSystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    CurrentWeatherCondition = EWeatherCondition::Clear;
    MaterialUpdateRadius = 5000.0f;
    bAutoUpdateMaterials = true;
    bDebugMaterialUpdates = false;
    PerformanceLevel = EMaterialPerformanceLevel::High;
    
    // Initialize current parameters
    CurrentParameters.WetnessLevel = 0.0f;
    CurrentParameters.MoistureLevel = 0.5f;
    CurrentParameters.Temperature = 0.3f;
    CurrentParameters.WindStrength = 0.3f;
    CurrentParameters.TimeOfDay = 0.5f;
    CurrentParameters.SeasonFactor = 0.7f;
    CurrentParameters.WearFactor = 0.1f;
    CurrentParameters.VegetationGrowth = 0.8f;
}

void UMaterialSystemComponent::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Initializing material system"));
    
    InitializeMaterialSystem();
    
    if (bAutoUpdateMaterials && MaterialConfig && MaterialConfig->bEnableRealTimeUpdates)
    {
        StartMaterialUpdates();
    }
}

void UMaterialSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopMaterialUpdates();
    Super::EndPlay(EndPlayReason);
}

void UMaterialSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoUpdateMaterials)
    {
        UpdateEnvironmentalFactors(DeltaTime);
    }
}

void UMaterialSystemComponent::InitializeMaterialSystem()
{
    if (!MaterialConfig)
    {
        UE_LOG(LogMaterialSystem, Warning, TEXT("MaterialSystemComponent: No MaterialConfig assigned"));
        return;
    }
    
    // Load material parameter collection
    if (MaterialConfig->ParameterCollection.IsValid())
    {
        ParameterCollectionInstance = GetWorld()->GetParameterCollectionInstance(MaterialConfig->ParameterCollection.Get());
        if (ParameterCollectionInstance)
        {
            UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Parameter collection loaded successfully"));
        }
    }
    
    // Initialize landscape materials
    InitializeLandscapeMaterials();
    
    // Set initial parameters
    CurrentParameters = MaterialConfig->DefaultParameters;
    UpdateMaterialParameters();
    
    UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Material system initialized"));
}

void UMaterialSystemComponent::InitializeLandscapeMaterials()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all landscape actors in the world
    for (TActorIterator<ALandscape> LandscapeIterator(World); LandscapeIterator; ++LandscapeIterator)
    {
        ALandscape* Landscape = *LandscapeIterator;
        if (Landscape && MaterialConfig && MaterialConfig->MasterLandscapeMaterial.IsValid())
        {
            // Set the master landscape material
            Landscape->LandscapeMaterial = MaterialConfig->MasterLandscapeMaterial.Get();
            
            // Configure landscape layer info objects
            ConfigureLandscapeLayers(Landscape);
            
            UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Configured landscape: %s"), *Landscape->GetName());
        }
    }
}

void UMaterialSystemComponent::ConfigureLandscapeLayers(ALandscape* Landscape)
{
    if (!Landscape || !MaterialConfig)
    {
        return;
    }
    
    // Set up landscape layer info objects
    for (int32 i = 0; i < MaterialConfig->LandscapeLayerInfos.Num(); ++i)
    {
        if (MaterialConfig->LandscapeLayerInfos[i].IsValid())
        {
            ULandscapeLayerInfoObject* LayerInfo = MaterialConfig->LandscapeLayerInfos[i].Get();
            if (LayerInfo)
            {
                // Add layer info to landscape
                Landscape->EditorLayerSettings.Add(FLandscapeEditorLayerSettings(LayerInfo));
                
                UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Added layer info: %s"), *LayerInfo->LayerName.ToString());
            }
        }
    }
}

void UMaterialSystemComponent::StartMaterialUpdates()
{
    if (!MaterialConfig)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().SetTimer(
            MaterialUpdateTimer,
            this,
            &UMaterialSystemComponent::UpdateMaterialParameters,
            MaterialConfig->UpdateFrequency,
            true
        );
        
        UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Started material updates (frequency: %.2f seconds)"), MaterialConfig->UpdateFrequency);
    }
}

void UMaterialSystemComponent::StopMaterialUpdates()
{
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(MaterialUpdateTimer);
        UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Stopped material updates"));
    }
}

void UMaterialSystemComponent::UpdateEnvironmentalFactors(float DeltaTime)
{
    // Update time of day
    CurrentParameters.TimeOfDay += DeltaTime / 86400.0f; // 24 hours in seconds
    if (CurrentParameters.TimeOfDay > 1.0f)
    {
        CurrentParameters.TimeOfDay -= 1.0f;
    }
    
    // Update weather-based parameters
    UpdateWeatherEffects(DeltaTime);
    
    // Update seasonal changes (very slow)
    UpdateSeasonalChanges(DeltaTime);
    
    // Update vegetation growth
    UpdateVegetationGrowth(DeltaTime);
}

void UMaterialSystemComponent::UpdateWeatherEffects(float DeltaTime)
{
    // Get target weather parameters
    FDynamicMaterialParams TargetParams = GetWeatherParameters(CurrentWeatherCondition);
    
    // Smoothly interpolate to target values
    float InterpolationSpeed = 2.0f; // Adjust for weather transition speed
    
    CurrentParameters.WetnessLevel = FMath::FInterpTo(
        CurrentParameters.WetnessLevel,
        TargetParams.WetnessLevel,
        DeltaTime,
        InterpolationSpeed
    );
    
    CurrentParameters.MoistureLevel = FMath::FInterpTo(
        CurrentParameters.MoistureLevel,
        TargetParams.MoistureLevel,
        DeltaTime,
        InterpolationSpeed * 0.5f // Moisture changes slower
    );
    
    CurrentParameters.WindStrength = FMath::FInterpTo(
        CurrentParameters.WindStrength,
        TargetParams.WindStrength,
        DeltaTime,
        InterpolationSpeed * 2.0f // Wind changes faster
    );
}

void UMaterialSystemComponent::UpdateSeasonalChanges(float DeltaTime)
{
    // Very slow seasonal progression (for demonstration)
    float SeasonalSpeed = 0.00001f; // Extremely slow for testing
    
    CurrentParameters.SeasonFactor += DeltaTime * SeasonalSpeed;
    if (CurrentParameters.SeasonFactor > 1.0f)
    {
        CurrentParameters.SeasonFactor -= 1.0f;
    }
}

void UMaterialSystemComponent::UpdateVegetationGrowth(float DeltaTime)
{
    // Vegetation responds to moisture and season
    float TargetGrowth = (CurrentParameters.MoistureLevel + CurrentParameters.SeasonFactor) * 0.5f;
    
    CurrentParameters.VegetationGrowth = FMath::FInterpTo(
        CurrentParameters.VegetationGrowth,
        TargetGrowth,
        DeltaTime,
        0.1f // Slow vegetation growth
    );
}

FDynamicMaterialParams UMaterialSystemComponent::GetWeatherParameters(EWeatherCondition WeatherCondition) const
{
    if (MaterialConfig && MaterialConfig->WeatherResponses.Contains(WeatherCondition))
    {
        return MaterialConfig->WeatherResponses[WeatherCondition];
    }
    
    // Default weather parameters
    FDynamicMaterialParams DefaultWeather;
    
    switch (WeatherCondition)
    {
        case EWeatherCondition::Clear:
            DefaultWeather.WetnessLevel = 0.0f;
            DefaultWeather.MoistureLevel = 0.4f;
            DefaultWeather.WindStrength = 0.2f;
            break;
            
        case EWeatherCondition::LightRain:
            DefaultWeather.WetnessLevel = 0.6f;
            DefaultWeather.MoistureLevel = 0.8f;
            DefaultWeather.WindStrength = 0.4f;
            break;
            
        case EWeatherCondition::HeavyRain:
            DefaultWeather.WetnessLevel = 1.0f;
            DefaultWeather.MoistureLevel = 1.0f;
            DefaultWeather.WindStrength = 0.7f;
            break;
            
        case EWeatherCondition::Fog:
            DefaultWeather.WetnessLevel = 0.3f;
            DefaultWeather.MoistureLevel = 0.9f;
            DefaultWeather.WindStrength = 0.1f;
            break;
            
        case EWeatherCondition::Storm:
            DefaultWeather.WetnessLevel = 1.0f;
            DefaultWeather.MoistureLevel = 1.0f;
            DefaultWeather.WindStrength = 1.0f;
            break;
            
        case EWeatherCondition::Drought:
            DefaultWeather.WetnessLevel = 0.0f;
            DefaultWeather.MoistureLevel = 0.1f;
            DefaultWeather.WindStrength = 0.3f;
            break;
            
        case EWeatherCondition::HighHumidity:
            DefaultWeather.WetnessLevel = 0.2f;
            DefaultWeather.MoistureLevel = 0.9f;
            DefaultWeather.WindStrength = 0.1f;
            break;
    }
    
    return DefaultWeather;
}

void UMaterialSystemComponent::UpdateMaterialParameters()
{
    if (!ParameterCollectionInstance)
    {
        return;
    }
    
    // Update scalar parameters
    ParameterCollectionInstance->SetScalarParameterValue(FName("WetnessLevel"), CurrentParameters.WetnessLevel);
    ParameterCollectionInstance->SetScalarParameterValue(FName("MoistureLevel"), CurrentParameters.MoistureLevel);
    ParameterCollectionInstance->SetScalarParameterValue(FName("Temperature"), CurrentParameters.Temperature);
    ParameterCollectionInstance->SetScalarParameterValue(FName("WindStrength"), CurrentParameters.WindStrength);
    ParameterCollectionInstance->SetScalarParameterValue(FName("TimeOfDay"), CurrentParameters.TimeOfDay);
    ParameterCollectionInstance->SetScalarParameterValue(FName("SeasonFactor"), CurrentParameters.SeasonFactor);
    ParameterCollectionInstance->SetScalarParameterValue(FName("WearFactor"), CurrentParameters.WearFactor);
    ParameterCollectionInstance->SetScalarParameterValue(FName("VegetationGrowth"), CurrentParameters.VegetationGrowth);
    
    // Update dynamic material instances
    UpdateDynamicMaterialInstances();
    
    if (bDebugMaterialUpdates)
    {
        UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Updated parameters - Wetness: %.2f, Moisture: %.2f, TimeOfDay: %.2f"), 
               CurrentParameters.WetnessLevel, CurrentParameters.MoistureLevel, CurrentParameters.TimeOfDay);
    }
}

void UMaterialSystemComponent::UpdateDynamicMaterialInstances()
{
    // Update any cached dynamic material instances
    for (auto& MaterialPair : DynamicMaterialInstances)
    {
        UMaterialInstanceDynamic* DynamicMaterial = MaterialPair.Value;
        if (DynamicMaterial)
        {
            UpdateDynamicMaterialInstance(DynamicMaterial);
        }
    }
}

void UMaterialSystemComponent::UpdateDynamicMaterialInstance(UMaterialInstanceDynamic* DynamicMaterial)
{
    if (!DynamicMaterial)
    {
        return;
    }
    
    // Set material parameters
    DynamicMaterial->SetScalarParameterValue(FName("WetnessLevel"), CurrentParameters.WetnessLevel);
    DynamicMaterial->SetScalarParameterValue(FName("MoistureLevel"), CurrentParameters.MoistureLevel);
    DynamicMaterial->SetScalarParameterValue(FName("Temperature"), CurrentParameters.Temperature);
    DynamicMaterial->SetScalarParameterValue(FName("WindStrength"), CurrentParameters.WindStrength);
    DynamicMaterial->SetScalarParameterValue(FName("TimeOfDay"), CurrentParameters.TimeOfDay);
    DynamicMaterial->SetScalarParameterValue(FName("SeasonFactor"), CurrentParameters.SeasonFactor);
    DynamicMaterial->SetScalarParameterValue(FName("WearFactor"), CurrentParameters.WearFactor);
    DynamicMaterial->SetScalarParameterValue(FName("VegetationGrowth"), CurrentParameters.VegetationGrowth);
}

void UMaterialSystemComponent::SetWeatherCondition(EWeatherCondition NewWeatherCondition)
{
    if (CurrentWeatherCondition != NewWeatherCondition)
    {
        CurrentWeatherCondition = NewWeatherCondition;
        
        UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Weather condition changed to: %d"), (int32)NewWeatherCondition);
        
        // Immediate update for weather changes
        if (bAutoUpdateMaterials)
        {
            UpdateMaterialParameters();
        }
    }
}

void UMaterialSystemComponent::SetTimeOfDay(float NewTimeOfDay)
{
    CurrentParameters.TimeOfDay = FMath::Clamp(NewTimeOfDay, 0.0f, 1.0f);
    
    if (bAutoUpdateMaterials)
    {
        UpdateMaterialParameters();
    }
}

void UMaterialSystemComponent::SetSeasonFactor(float NewSeasonFactor)
{
    CurrentParameters.SeasonFactor = FMath::Clamp(NewSeasonFactor, 0.0f, 1.0f);
    
    if (bAutoUpdateMaterials)
    {
        UpdateMaterialParameters();
    }
}

UMaterialInstanceDynamic* UMaterialSystemComponent::CreateDynamicMaterialInstance(UMaterialInterface* BaseMaterial, const FString& InstanceName)
{
    if (!BaseMaterial)
    {
        return nullptr;
    }
    
    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
    if (DynamicMaterial)
    {
        // Cache the dynamic material instance
        DynamicMaterialInstances.Add(InstanceName, DynamicMaterial);
        
        // Apply current parameters
        UpdateDynamicMaterialInstance(DynamicMaterial);
        
        UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Created dynamic material instance: %s"), *InstanceName);
    }
    
    return DynamicMaterial;
}

void UMaterialSystemComponent::ApplyMaterialToStaticMesh(UStaticMeshComponent* MeshComponent, const FString& MaterialConfigName)
{
    if (!MeshComponent || !MaterialConfig)
    {
        return;
    }
    
    // Find material layer by name
    for (const FMaterialLayerData& Layer : MaterialConfig->MaterialLayers)
    {
        if (Layer.LayerName == MaterialConfigName && Layer.BaseMaterial.IsValid())
        {
            UMaterialInterface* BaseMaterial = Layer.BaseMaterial.Get();
            UMaterialInstanceDynamic* DynamicMaterial = CreateDynamicMaterialInstance(BaseMaterial, MaterialConfigName);
            
            if (DynamicMaterial)
            {
                MeshComponent->SetMaterial(0, DynamicMaterial);
                UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Applied material %s to mesh %s"), 
                       *MaterialConfigName, *MeshComponent->GetName());
            }
            break;
        }
    }
}

void UMaterialSystemComponent::SetMaterialConfig(UMaterialConfigData* NewMaterialConfig)
{
    if (MaterialConfig != NewMaterialConfig)
    {
        MaterialConfig = NewMaterialConfig;
        
        if (MaterialConfig)
        {
            // Reinitialize with new config
            InitializeMaterialSystem();
            UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Material config changed to: %s"), 
                   *MaterialConfig->ConfigName.ToString());
        }
    }
}

FDynamicMaterialParams UMaterialSystemComponent::GetCurrentMaterialParameters() const
{
    return CurrentParameters;
}

void UMaterialSystemComponent::SetDebugMode(bool bEnabled)
{
    bDebugMaterialUpdates = bEnabled;
    UE_LOG(LogMaterialSystem, Log, TEXT("MaterialSystemComponent: Debug mode %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}