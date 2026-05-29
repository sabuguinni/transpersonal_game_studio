#include "LightingSystemManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

ULightingSystemManager::ULightingSystemManager()
{
    SunLight = nullptr;
    SkyAtmosphereActor = nullptr;
    FogActor = nullptr;
    PostProcessVolume = nullptr;
    CurrentTimeOfDay = 12.0f;
}

void ULightingSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager initialized"));
    
    // Initialize with default Cretaceous tropical lighting
    CurrentLightingSettings = FLight_TimeOfDaySettings();
    CurrentWeatherSettings = FLight_WeatherSettings();
    
    // Delay initialization to ensure world is ready
    if (UWorld* World = GetWorld())
    {
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, this, &ULightingSystemManager::InitializeLightingSystem, 1.0f, false);
    }
}

void ULightingSystemManager::Deinitialize()
{
    SunLight = nullptr;
    SkyAtmosphereActor = nullptr;
    FogActor = nullptr;
    PostProcessVolume = nullptr;
    Super::Deinitialize();
}

void ULightingSystemManager::InitializeLightingSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Initializing Lighting System"));
    
    FindOrCreateSunLight();
    FindOrCreateSkyAtmosphere();
    FindOrCreateFog();
    ConfigurePostProcessing();
    
    // Apply default Cretaceous atmospheric correction
    ApplyAtmosphericCorrection();
    
    UE_LOG(LogTemp, Warning, TEXT("Lighting System initialization complete"));
}

void ULightingSystemManager::ApplyAtmosphericCorrection()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying atmospheric correction for Cretaceous period"));
    
    // Configure sun for warm tropical lighting
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CurrentLightingSettings.SunIntensity);
        LightComp->SetLightColor(CurrentLightingSettings.SunColor);
        LightComp->SetTemperature(CurrentLightingSettings.SunTemperature);
        LightComp->SetCastShadows(true);
        LightComp->SetCastVolumetricShadow(true);
        
        // Set sun angle for tropical midday
        SunLight->SetActorRotation(CurrentLightingSettings.SunRotation);
        
        UE_LOG(LogTemp, Warning, TEXT("Sun lighting configured: Intensity=%.1f, Temp=%.0fK"), 
               CurrentLightingSettings.SunIntensity, CurrentLightingSettings.SunTemperature);
    }
    
    // Configure atmosphere for clear tropical sky
    if (SkyAtmosphereActor)
    {
        if (USkyAtmosphereComponent* SkyComp = SkyAtmosphereActor->GetComponent<USkyAtmosphereComponent>())
        {
            SkyComp->SetRayleighScatteringScale(CurrentLightingSettings.RayleighScattering);
            SkyComp->SetMieScatteringScale(CurrentLightingSettings.MieScattering);
            SkyComp->SetMieAbsorptionScale(0.01f);
            SkyComp->SetOtherAbsorptionScale(0.1f);
            
            UE_LOG(LogTemp, Warning, TEXT("Sky atmosphere configured for clear tropical conditions"));
        }
    }
    
    // Configure fog for light atmospheric haze
    if (FogActor)
    {
        if (UExponentialHeightFogComponent* FogComp = FogActor->GetComponent<UExponentialHeightFogComponent>())
        {
            FogComp->SetFogDensity(CurrentLightingSettings.FogDensity);
            FogComp->SetFogHeightFalloff(CurrentLightingSettings.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(FLinearColor(0.9f, 0.95f, 1.0f, 1.0f));
            
            UE_LOG(LogTemp, Warning, TEXT("Fog configured: Density=%.3f"), CurrentLightingSettings.FogDensity);
        }
    }
    
    UpdatePostProcessing();
}

void ULightingSystemManager::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
    
    // Update lighting based on time
    CurrentLightingSettings.SunIntensity = InterpolateSunIntensity(TimeHours);
    CurrentLightingSettings.SunColor = InterpolateSunColor(TimeHours);
    CurrentLightingSettings.SunRotation = CalculateSunRotation(TimeHours);
    
    UpdateSunLighting();
    UpdateAtmosphere();
    
    UE_LOG(LogTemp, Log, TEXT("Time of day set to %.1f hours"), TimeHours);
}

void ULightingSystemManager::SetWeatherConditions(const FLight_WeatherSettings& WeatherSettings)
{
    CurrentWeatherSettings = WeatherSettings;
    
    // Apply weather effects to lighting
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        // Reduce sun intensity based on cloud coverage
        float WeatherIntensity = CurrentLightingSettings.SunIntensity * (1.0f - CurrentWeatherSettings.CloudCoverage * 0.7f);
        LightComp->SetIntensity(WeatherIntensity);
        
        // Apply weather tint
        FLinearColor WeatherColor = CurrentLightingSettings.SunColor * CurrentWeatherSettings.WeatherTint;
        LightComp->SetLightColor(WeatherColor);
    }
    
    // Update fog for weather conditions
    if (FogActor)
    {
        if (UExponentialHeightFogComponent* FogComp = FogActor->GetComponent<UExponentialHeightFogComponent>())
        {
            // Increase fog density in rain/storms
            float WeatherFogDensity = CurrentLightingSettings.FogDensity * (1.0f + CurrentWeatherSettings.RainIntensity * 2.0f);
            FogComp->SetFogDensity(WeatherFogDensity);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Weather conditions applied: Clouds=%.1f, Rain=%.1f"), 
           WeatherSettings.CloudCoverage, WeatherSettings.RainIntensity);
}

void ULightingSystemManager::FindOrCreateSunLight()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        UE_LOG(LogTemp, Warning, TEXT("Found existing DirectionalLight"));
        return;
    }
    
    // Create new directional light if none exists
    FVector SpawnLocation(0.0f, 0.0f, 500.0f);
    FRotator SpawnRotation(-45.0f, 45.0f, 0.0f);
    
    SunLight = World->SpawnActor<ADirectionalLight>(SpawnLocation, SpawnRotation);
    if (SunLight)
    {
        SunLight->SetActorLabel(TEXT("Sun_DirectionalLight"));
        UE_LOG(LogTemp, Warning, TEXT("Created new DirectionalLight"));
    }
}

void ULightingSystemManager::FindOrCreateSkyAtmosphere()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphereActor = *ActorItr;
        UE_LOG(LogTemp, Warning, TEXT("Found existing SkyAtmosphere"));
        return;
    }
    
    // Create new sky atmosphere if none exists
    SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphere>();
    if (SkyAtmosphereActor)
    {
        SkyAtmosphereActor->SetActorLabel(TEXT("Cretaceous_SkyAtmosphere"));
        UE_LOG(LogTemp, Warning, TEXT("Created new SkyAtmosphere"));
    }
}

void ULightingSystemManager::FindOrCreateFog()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing fog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActor = *ActorItr;
        UE_LOG(LogTemp, Warning, TEXT("Found existing ExponentialHeightFog"));
        return;
    }
    
    // Create new fog if none exists
    FogActor = World->SpawnActor<AExponentialHeightFog>();
    if (FogActor)
    {
        FogActor->SetActorLabel(TEXT("Atmospheric_Fog"));
        UE_LOG(LogTemp, Warning, TEXT("Created new ExponentialHeightFog"));
    }
}

void ULightingSystemManager::ConfigurePostProcessing()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing post process volume
    for (TActorIterator<APostProcessVolume> ActorItr(World); ActorItr; ++ActorItr)
    {
        PostProcessVolume = *ActorItr;
        UE_LOG(LogTemp, Warning, TEXT("Found existing PostProcessVolume"));
        break;
    }
    
    // Create if none exists
    if (!PostProcessVolume)
    {
        PostProcessVolume = World->SpawnActor<APostProcessVolume>();
        if (PostProcessVolume)
        {
            PostProcessVolume->SetActorLabel(TEXT("Global_PostProcess"));
            PostProcessVolume->bUnbound = true;
            UE_LOG(LogTemp, Warning, TEXT("Created new PostProcessVolume"));
        }
    }
    
    UpdatePostProcessing();
}

void ULightingSystemManager::ApplyBiomeLighting(EBiomeType BiomeType, const FVector& PlayerLocation)
{
    FLight_TimeOfDaySettings BiomeSettings = GetBiomeLightingSettings(BiomeType);
    
    // Blend current settings with biome-specific settings
    CurrentLightingSettings.SunIntensity = FMath::Lerp(CurrentLightingSettings.SunIntensity, BiomeSettings.SunIntensity, 0.1f);
    CurrentLightingSettings.FogDensity = FMath::Lerp(CurrentLightingSettings.FogDensity, BiomeSettings.FogDensity, 0.1f);
    
    UpdateSunLighting();
    UpdateFog();
    
    UE_LOG(LogTemp, Log, TEXT("Applied biome lighting for type: %d"), (int32)BiomeType);
}

void ULightingSystemManager::SetInteriorLighting(bool bIsInterior, float AmbientMultiplier)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        
        if (bIsInterior)
        {
            // Reduce sun intensity for interior spaces
            LightComp->SetIntensity(CurrentLightingSettings.SunIntensity * AmbientMultiplier);
            UE_LOG(LogTemp, Log, TEXT("Interior lighting applied: multiplier=%.2f"), AmbientMultiplier);
        }
        else
        {
            // Restore full sun intensity for exterior
            LightComp->SetIntensity(CurrentLightingSettings.SunIntensity);
            UE_LOG(LogTemp, Log, TEXT("Exterior lighting restored"));
        }
    }
}

void ULightingSystemManager::UpdateSunLighting()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(CurrentLightingSettings.SunIntensity);
        LightComp->SetLightColor(CurrentLightingSettings.SunColor);
        LightComp->SetTemperature(CurrentLightingSettings.SunTemperature);
        SunLight->SetActorRotation(CurrentLightingSettings.SunRotation);
    }
}

void ULightingSystemManager::UpdateAtmosphere()
{
    if (SkyAtmosphereActor)
    {
        if (USkyAtmosphereComponent* SkyComp = SkyAtmosphereActor->GetComponent<USkyAtmosphereComponent>())
        {
            SkyComp->SetRayleighScatteringScale(CurrentLightingSettings.RayleighScattering);
            SkyComp->SetMieScatteringScale(CurrentLightingSettings.MieScattering);
        }
    }
}

void ULightingSystemManager::UpdateFog()
{
    if (FogActor)
    {
        if (UExponentialHeightFogComponent* FogComp = FogActor->GetComponent<UExponentialHeightFogComponent>())
        {
            FogComp->SetFogDensity(CurrentLightingSettings.FogDensity);
            FogComp->SetFogHeightFalloff(CurrentLightingSettings.FogHeightFalloff);
        }
    }
}

void ULightingSystemManager::UpdatePostProcessing()
{
    if (PostProcessVolume)
    {
        if (UPostProcessComponent* PPComp = PostProcessVolume->GetComponent<UPostProcessComponent>())
        {
            FPostProcessSettings& Settings = PPComp->Settings;
            
            // Tropical daylight post-processing
            Settings.bOverride_AutoExposureBias = true;
            Settings.AutoExposureBias = 0.5f;
            
            Settings.bOverride_WhiteTemp = true;
            Settings.WhiteTemp = 6500.0f;
            
            Settings.bOverride_WhiteTint = true;
            Settings.WhiteTint = 0.0f;
            
            UE_LOG(LogTemp, Log, TEXT("Post-processing updated for tropical conditions"));
        }
    }
}

FLight_TimeOfDaySettings ULightingSystemManager::GetBiomeLightingSettings(EBiomeType BiomeType) const
{
    FLight_TimeOfDaySettings BiomeSettings = CurrentLightingSettings;
    
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
            BiomeSettings.FogDensity = 0.08f;
            BiomeSettings.SunIntensity = 3.5f;
            break;
        case EBiomeType::Forest:
            BiomeSettings.SunIntensity = 2.5f;
            BiomeSettings.FogDensity = 0.04f;
            break;
        case EBiomeType::Desert:
            BiomeSettings.SunIntensity = 7.0f;
            BiomeSettings.FogDensity = 0.01f;
            break;
        case EBiomeType::Mountain:
            BiomeSettings.SunIntensity = 6.0f;
            BiomeSettings.FogDensity = 0.03f;
            break;
        case EBiomeType::Savanna:
        default:
            // Use default tropical savanna settings
            break;
    }
    
    return BiomeSettings;
}

FLinearColor ULightingSystemManager::InterpolateSunColor(float TimeHours) const
{
    // Sunrise/sunset warm colors vs midday neutral
    if (TimeHours < 6.0f || TimeHours > 18.0f)
    {
        // Night - cool blue
        return FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    }
    else if (TimeHours < 8.0f || TimeHours > 16.0f)
    {
        // Sunrise/sunset - warm orange
        return FLinearColor(1.0f, 0.7f, 0.4f, 1.0f);
    }
    else
    {
        // Midday - warm white
        return FLinearColor(1.0f, 0.95f, 0.85f, 1.0f);
    }
}

float ULightingSystemManager::InterpolateSunIntensity(float TimeHours) const
{
    // Peak intensity at noon, minimum at midnight
    float NormalizedTime = FMath::Abs(TimeHours - 12.0f) / 12.0f;
    return FMath::Lerp(5.0f, 0.1f, NormalizedTime);
}

FRotator ULightingSystemManager::CalculateSunRotation(float TimeHours) const
{
    // Simple sun arc calculation
    float SunAngle = (TimeHours - 6.0f) * 15.0f - 90.0f; // 15 degrees per hour
    return FRotator(SunAngle, 45.0f, 0.0f);
}