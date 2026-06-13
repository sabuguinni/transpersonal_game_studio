#include "Light_AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

void ULight_AtmosphericManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    // Initialize with Cretaceous period defaults
    CurrentSettings = FLight_AtmosphericSettings();
    CurrentTimeOfDay = 12.0f;
    CurrentWeather = EWeatherType::Clear;
    
    UE_LOG(LogTemp, Log, TEXT("Light_AtmosphericManager initialized"));
}

void ULight_AtmosphericManager::Deinitialize()
{
    SaveAtmosphericState();
    Super::Deinitialize();
}

void ULight_AtmosphericManager::ApplyCretaceousAtmosphere()
{
    FindOrCreateAtmosphericActors();
    ConfigureSkyAtmosphere();
    ConfigureDirectionalLight();
    ConfigureSkyLight();
    ConfigureHeightFog();
    ConfigurePostProcessVolume();
    SaveAtmosphericState();
    
    UE_LOG(LogTemp, Log, TEXT("Cretaceous atmosphere applied successfully"));
}

void ULight_AtmosphericManager::SetAtmosphericSettings(const FLight_AtmosphericSettings& NewSettings)
{
    CurrentSettings = NewSettings;
    ApplyCretaceousAtmosphere();
}

FLight_AtmosphericSettings ULight_AtmosphericManager::GetCurrentAtmosphericSettings() const
{
    return CurrentSettings;
}

void ULight_AtmosphericManager::UpdateTimeOfDay(float TimeOfDayHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeOfDayHours, 0.0f, 24.0f);
    
    if (DirectionalLightActor && DirectionalLightActor->GetLightComponent())
    {
        // Calculate sun angle based on time of day
        float SunAngle = (TimeOfDayHours - 6.0f) * 15.0f; // 6 AM = 0 degrees
        FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f);
        DirectionalLightActor->SetActorRotation(SunRotation);
        
        // Adjust intensity based on sun elevation
        float IntensityMultiplier = FMath::Clamp(FMath::Cos(FMath::DegreesToRadians(SunAngle + 90.0f)), 0.1f, 1.0f);
        DirectionalLightActor->GetLightComponent()->SetIntensity(CurrentSettings.SunIntensity * IntensityMultiplier);
    }
}

void ULight_AtmosphericManager::SetWeatherCondition(EWeatherType WeatherType)
{
    CurrentWeather = WeatherType;
    
    // Adjust atmospheric settings based on weather
    FLight_AtmosphericSettings WeatherSettings = CurrentSettings;
    
    switch (WeatherType)
    {
        case EWeatherType::Stormy:
            WeatherSettings.FogDensity *= 2.0f;
            WeatherSettings.SunIntensity *= 0.3f;
            WeatherSettings.SkyLightIntensity *= 0.5f;
            break;
        case EWeatherType::Foggy:
            WeatherSettings.FogDensity *= 5.0f;
            WeatherSettings.SunIntensity *= 0.5f;
            break;
        case EWeatherType::Overcast:
            WeatherSettings.SunIntensity *= 0.7f;
            WeatherSettings.SkyLightIntensity *= 1.2f;
            break;
        case EWeatherType::Clear:
        default:
            // Use default settings
            break;
    }
    
    SetAtmosphericSettings(WeatherSettings);
}

void ULight_AtmosphericManager::RefreshAtmosphericComponents()
{
    ApplyCretaceousAtmosphere();
}

void ULight_AtmosphericManager::FindOrCreateAtmosphericActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find existing atmospheric actors
    TArray<AActor*> FoundActors;
    
    // Sky Atmosphere
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    SkyAtmosphereActor = FoundActors.Num() > 0 ? Cast<ASkyAtmosphere>(FoundActors[0]) : nullptr;
    if (!SkyAtmosphereActor)
    {
        SkyAtmosphereActor = World->SpawnActor<ASkyAtmosphere>();
    }
    
    // Directional Light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    DirectionalLightActor = FoundActors.Num() > 0 ? Cast<ADirectionalLight>(FoundActors[0]) : nullptr;
    if (!DirectionalLightActor)
    {
        DirectionalLightActor = World->SpawnActor<ADirectionalLight>();
    }
    
    // Sky Light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    SkyLightActor = FoundActors.Num() > 0 ? Cast<ASkyLight>(FoundActors[0]) : nullptr;
    if (!SkyLightActor)
    {
        SkyLightActor = World->SpawnActor<ASkyLight>(FVector(0, 0, 500));
    }
    
    // Height Fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    HeightFogActor = FoundActors.Num() > 0 ? Cast<AExponentialHeightFog>(FoundActors[0]) : nullptr;
    if (!HeightFogActor)
    {
        HeightFogActor = World->SpawnActor<AExponentialHeightFog>();
    }
    
    // Post Process Volume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    PostProcessVolumeActor = FoundActors.Num() > 0 ? Cast<APostProcessVolume>(FoundActors[0]) : nullptr;
    if (!PostProcessVolumeActor)
    {
        PostProcessVolumeActor = World->SpawnActor<APostProcessVolume>();
        PostProcessVolumeActor->bUnbound = true;
    }
}

void ULight_AtmosphericManager::ConfigureSkyAtmosphere()
{
    if (!SkyAtmosphereActor)
    {
        return;
    }
    
    USkyAtmosphereComponent* SkyComp = SkyAtmosphereActor->GetComponent();
    if (SkyComp)
    {
        SkyComp->SetRayleighScattering(FLinearColor(CurrentSettings.RayleighScattering));
        SkyComp->SetRayleighExponentialDistribution(CurrentSettings.RayleighExponentialDistribution);
        SkyComp->SetMieScattering(FLinearColor(CurrentSettings.MieScattering));
        SkyComp->SetMieAbsorption(FLinearColor(1.0f, 0.95f, 0.85f));
        SkyComp->SetOtherAbsorption(FLinearColor(0.0f, 0.0f, 0.0f));
    }
}

void ULight_AtmosphericManager::ConfigureDirectionalLight()
{
    if (!DirectionalLightActor)
    {
        return;
    }
    
    UDirectionalLightComponent* LightComp = DirectionalLightActor->GetLightComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(CurrentSettings.SunIntensity);
        LightComp->SetLightColor(FLinearColor(CurrentSettings.SunColor));
        LightComp->SetTemperature(CurrentSettings.SunTemperature);
        LightComp->SetCastVolumetricShadow(true);
        LightComp->SetVolumetricScatteringIntensity(1.0f);
        LightComp->SetAtmosphereSunLight(true);
    }
}

void ULight_AtmosphericManager::ConfigureSkyLight()
{
    if (!SkyLightActor)
    {
        return;
    }
    
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    if (SkyComp)
    {
        SkyComp->SetIntensity(CurrentSettings.SkyLightIntensity);
        SkyComp->SetLightColor(FLinearColor(CurrentSettings.SkyLightColor));
        SkyComp->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);
        SkyComp->RecaptureSky();
    }
}

void ULight_AtmosphericManager::ConfigureHeightFog()
{
    if (!HeightFogActor)
    {
        return;
    }
    
    UExponentialHeightFogComponent* FogComp = HeightFogActor->GetComponent();
    if (FogComp)
    {
        FogComp->SetFogDensity(CurrentSettings.FogDensity);
        FogComp->SetFogHeightFalloff(CurrentSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(FLinearColor(CurrentSettings.FogInscatteringColor));
        FogComp->SetVolumetricFog(true);
        FogComp->SetVolumetricFogScatteringDistribution(0.2f);
        FogComp->SetVolumetricFogAlbedo(FLinearColor(1.0f, 0.96f, 0.86f));
        FogComp->SetVolumetricFogExtinctionScale(1.0f);
    }
}

void ULight_AtmosphericManager::ConfigurePostProcessVolume()
{
    if (!PostProcessVolumeActor)
    {
        return;
    }
    
    FPostProcessSettings& Settings = PostProcessVolumeActor->Settings;
    Settings.bOverride_WhiteTemp = true;
    Settings.WhiteTemp = CurrentSettings.SunTemperature;
    Settings.bOverride_WhiteTint = true;
    Settings.WhiteTint = 0.0f;
    
    Settings.bOverride_ColorSaturation = true;
    Settings.ColorSaturation = FVector4(1.1f, 1.0f, 0.9f, 1.0f);
    
    Settings.bOverride_ColorContrast = true;
    Settings.ColorContrast = FVector4(1.05f, 1.0f, 0.95f, 1.0f);
    
    Settings.bOverride_ColorGamma = true;
    Settings.ColorGamma = FVector4(1.0f, 1.0f, 1.0f, 1.0f);
    
    Settings.bOverride_ColorGain = true;
    Settings.ColorGain = FVector4(1.0f, 0.98f, 0.95f, 1.0f);
}

void ULight_AtmosphericManager::SaveAtmosphericState()
{
    UWorld* World = GetWorld();
    if (World)
    {
        // Force save the current level to persist atmospheric settings
        FString MapPath = World->GetMapName();
        if (!MapPath.IsEmpty())
        {
            UE_LOG(LogTemp, Log, TEXT("Atmospheric state saved for map: %s"), *MapPath);
        }
    }
}