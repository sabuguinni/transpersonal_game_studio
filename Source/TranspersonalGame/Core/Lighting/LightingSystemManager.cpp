#include "LightingSystemManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ULightingSystemManager::ULightingSystemManager()
{
    SunLight = nullptr;
    SkyLightActor = nullptr;
    SkyAtmosphereActor = nullptr;
    FogActor = nullptr;
    CurrentTimeOfDay = ELight_TimeOfDay::Midday;
    CurrentWeatherType = ELight_WeatherType::Clear;
    TimeOfDayProgress = 0.5f;
    bDayNightCycleEnabled = true;
    DayNightCycleSpeed = 1.0f;
}

void ULightingSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Initialized"));
}

void ULightingSystemManager::Deinitialize()
{
    Super::Deinitialize();
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Deinitialized"));
}

bool ULightingSystemManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void ULightingSystemManager::InitializeLightingSystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("LightingSystemManager: No valid world found"));
        return;
    }

    FindOrCreateLightingActors();
    ApplyLightingSettings(GetLightingSettingsForTimeOfDay(CurrentTimeOfDay));
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Lighting system initialized successfully"));
}

void ULightingSystemManager::FindOrCreateLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing directional light
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }

    // Find existing sky light
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }

    // Find existing sky atmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
    }

    // Find existing fog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }

    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found lighting actors - Sun: %s, Sky: %s, Atmosphere: %s, Fog: %s"),
        SunLight ? TEXT("Yes") : TEXT("No"),
        SkyLightActor ? TEXT("Yes") : TEXT("No"),
        SkyAtmosphereActor ? TEXT("Yes") : TEXT("No"),
        FogActor ? TEXT("Yes") : TEXT("No"));
}

void ULightingSystemManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    if (CurrentTimeOfDay != NewTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        
        // Update progress based on time of day
        switch (NewTimeOfDay)
        {
            case ELight_TimeOfDay::Dawn: TimeOfDayProgress = 0.0f; break;
            case ELight_TimeOfDay::Morning: TimeOfDayProgress = 0.125f; break;
            case ELight_TimeOfDay::Midday: TimeOfDayProgress = 0.25f; break;
            case ELight_TimeOfDay::Afternoon: TimeOfDayProgress = 0.375f; break;
            case ELight_TimeOfDay::Sunset: TimeOfDayProgress = 0.5f; break;
            case ELight_TimeOfDay::Dusk: TimeOfDayProgress = 0.625f; break;
            case ELight_TimeOfDay::Night: TimeOfDayProgress = 0.75f; break;
            case ELight_TimeOfDay::Midnight: TimeOfDayProgress = 0.875f; break;
        }

        ApplyLightingSettings(GetLightingSettingsForTimeOfDay(NewTimeOfDay));
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Time of day changed to %d"), (int32)NewTimeOfDay);
    }
}

void ULightingSystemManager::SetWeatherType(ELight_WeatherType NewWeatherType)
{
    if (CurrentWeatherType != NewWeatherType)
    {
        CurrentWeatherType = NewWeatherType;
        
        // Blend current time settings with weather settings
        FLight_LightingSettings TimeSettings = GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
        FLight_LightingSettings WeatherSettings = GetLightingSettingsForWeather(NewWeatherType);
        FLight_LightingSettings BlendedSettings = BlendLightingSettings(TimeSettings, WeatherSettings, 0.5f);
        
        ApplyLightingSettings(BlendedSettings);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Weather changed to %d"), (int32)NewWeatherType);
    }
}

void ULightingSystemManager::UpdateDayNightCycle(float DeltaTime)
{
    if (!bDayNightCycleEnabled)
    {
        return;
    }

    TimeOfDayProgress += DeltaTime * DayNightCycleSpeed * 0.001f; // Very slow cycle for testing
    if (TimeOfDayProgress >= 1.0f)
    {
        TimeOfDayProgress = 0.0f;
    }

    // Determine current time of day based on progress
    ELight_TimeOfDay NewTimeOfDay = ELight_TimeOfDay::Dawn;
    if (TimeOfDayProgress < 0.125f) NewTimeOfDay = ELight_TimeOfDay::Dawn;
    else if (TimeOfDayProgress < 0.25f) NewTimeOfDay = ELight_TimeOfDay::Morning;
    else if (TimeOfDayProgress < 0.375f) NewTimeOfDay = ELight_TimeOfDay::Midday;
    else if (TimeOfDayProgress < 0.5f) NewTimeOfDay = ELight_TimeOfDay::Afternoon;
    else if (TimeOfDayProgress < 0.625f) NewTimeOfDay = ELight_TimeOfDay::Sunset;
    else if (TimeOfDayProgress < 0.75f) NewTimeOfDay = ELight_TimeOfDay::Dusk;
    else if (TimeOfDayProgress < 0.875f) NewTimeOfDay = ELight_TimeOfDay::Night;
    else NewTimeOfDay = ELight_TimeOfDay::Midnight;

    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTimeOfDay);
    }
}

void ULightingSystemManager::ApplyLightingSettings(const FLight_LightingSettings& Settings)
{
    UpdateSunLighting();
    UpdateSkyLighting();
    UpdateFogSettings();
    UpdateAtmosphere();
}

void ULightingSystemManager::UpdateSunLighting()
{
    if (!SunLight || !SunLight->GetLightComponent())
    {
        return;
    }

    FLight_LightingSettings Settings = GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
    UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
    
    LightComp->SetLightColor(Settings.SunColor);
    LightComp->SetIntensity(Settings.SunIntensity);
    
    // Set sun angle based on time of day
    FRotator SunRotation = SunLight->GetActorRotation();
    SunRotation.Pitch = Settings.SunAngle;
    SunLight->SetActorRotation(SunRotation);
}

void ULightingSystemManager::UpdateSkyLighting()
{
    if (!SkyLightActor || !SkyLightActor->GetLightComponent())
    {
        return;
    }

    FLight_LightingSettings Settings = GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
    USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
    
    SkyComp->SetLightColor(Settings.SkyColor);
    SkyComp->SetIntensity(Settings.SkyIntensity);
    SkyComp->RecaptureSky();
}

void ULightingSystemManager::UpdateFogSettings()
{
    if (!FogActor || !FogActor->GetComponent())
    {
        return;
    }

    FLight_LightingSettings Settings = GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
    UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
    
    FogComp->SetFogInscatteringColor(Settings.FogColor);
    FogComp->SetFogDensity(Settings.FogDensity);
    FogComp->SetStartDistance(Settings.FogStartDistance);
}

void ULightingSystemManager::UpdateAtmosphere()
{
    if (!SkyAtmosphereActor)
    {
        return;
    }

    // Atmosphere updates can be added here for more advanced effects
}

FLight_LightingSettings ULightingSystemManager::GetCurrentLightingSettings() const
{
    return GetLightingSettingsForTimeOfDay(CurrentTimeOfDay);
}

FLight_LightingSettings ULightingSystemManager::GetLightingSettingsForTimeOfDay(ELight_TimeOfDay TimeOfDay) const
{
    FLight_LightingSettings Settings;
    
    switch (TimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            Settings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
            Settings.SunIntensity = 1.5f;
            Settings.SunAngle = 10.0f;
            Settings.SkyColor = FLinearColor(0.8f, 0.4f, 0.2f, 1.0f);
            Settings.SkyIntensity = 0.3f;
            break;
            
        case ELight_TimeOfDay::Morning:
            Settings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            Settings.SunIntensity = 2.5f;
            Settings.SunAngle = 25.0f;
            Settings.SkyColor = FLinearColor(0.6f, 0.7f, 0.9f, 1.0f);
            Settings.SkyIntensity = 0.6f;
            break;
            
        case ELight_TimeOfDay::Midday:
            Settings.SunColor = FLinearColor::White;
            Settings.SunIntensity = 4.0f;
            Settings.SunAngle = 60.0f;
            Settings.SkyColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);
            Settings.SkyIntensity = 1.0f;
            break;
            
        case ELight_TimeOfDay::Afternoon:
            Settings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
            Settings.SunIntensity = 3.0f;
            Settings.SunAngle = 45.0f;
            Settings.SkyColor = FLinearColor(0.4f, 0.6f, 0.8f, 1.0f);
            Settings.SkyIntensity = 0.8f;
            break;
            
        case ELight_TimeOfDay::Sunset:
            Settings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
            Settings.SunIntensity = 2.0f;
            Settings.SunAngle = 15.0f;
            Settings.SkyColor = FLinearColor(0.9f, 0.3f, 0.1f, 1.0f);
            Settings.SkyIntensity = 0.4f;
            break;
            
        case ELight_TimeOfDay::Dusk:
            Settings.SunColor = FLinearColor(0.8f, 0.3f, 0.1f, 1.0f);
            Settings.SunIntensity = 0.8f;
            Settings.SunAngle = 5.0f;
            Settings.SkyColor = FLinearColor(0.3f, 0.2f, 0.4f, 1.0f);
            Settings.SkyIntensity = 0.2f;
            break;
            
        case ELight_TimeOfDay::Night:
            Settings.SunColor = FLinearColor(0.1f, 0.1f, 0.3f, 1.0f);
            Settings.SunIntensity = 0.1f;
            Settings.SunAngle = -10.0f;
            Settings.SkyColor = FLinearColor(0.05f, 0.05f, 0.15f, 1.0f);
            Settings.SkyIntensity = 0.1f;
            break;
            
        case ELight_TimeOfDay::Midnight:
            Settings.SunColor = FLinearColor(0.05f, 0.05f, 0.2f, 1.0f);
            Settings.SunIntensity = 0.05f;
            Settings.SunAngle = -20.0f;
            Settings.SkyColor = FLinearColor(0.02f, 0.02f, 0.1f, 1.0f);
            Settings.SkyIntensity = 0.05f;
            break;
    }
    
    return Settings;
}

FLight_LightingSettings ULightingSystemManager::GetLightingSettingsForWeather(ELight_WeatherType WeatherType) const
{
    FLight_LightingSettings Settings;
    
    switch (WeatherType)
    {
        case ELight_WeatherType::Clear:
            // Use default settings
            break;
            
        case ELight_WeatherType::Cloudy:
            Settings.SunIntensity = 0.7f;
            Settings.SkyIntensity = 0.5f;
            Settings.FogDensity = 0.05f;
            break;
            
        case ELight_WeatherType::Rainy:
            Settings.SunIntensity = 0.4f;
            Settings.SkyIntensity = 0.3f;
            Settings.FogDensity = 0.08f;
            Settings.SkyColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);
            break;
            
        case ELight_WeatherType::Stormy:
            Settings.SunIntensity = 0.2f;
            Settings.SkyIntensity = 0.2f;
            Settings.FogDensity = 0.1f;
            Settings.SkyColor = FLinearColor(0.2f, 0.2f, 0.3f, 1.0f);
            break;
            
        case ELight_WeatherType::Foggy:
            Settings.SunIntensity = 0.3f;
            Settings.SkyIntensity = 0.2f;
            Settings.FogDensity = 0.2f;
            Settings.FogStartDistance = 100.0f;
            break;
            
        case ELight_WeatherType::Volcanic:
            Settings.SunIntensity = 0.5f;
            Settings.SkyIntensity = 0.3f;
            Settings.FogDensity = 0.15f;
            Settings.SkyColor = FLinearColor(0.6f, 0.3f, 0.1f, 1.0f);
            Settings.FogColor = FLinearColor(0.5f, 0.2f, 0.1f, 1.0f);
            break;
    }
    
    return Settings;
}

FLight_LightingSettings ULightingSystemManager::BlendLightingSettings(const FLight_LightingSettings& A, const FLight_LightingSettings& B, float Alpha) const
{
    FLight_LightingSettings Result;
    
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunAngle = FMath::Lerp(A.SunAngle, B.SunAngle, Alpha);
    Result.SkyColor = FLinearColor::LerpUsingHSV(A.SkyColor, B.SkyColor, Alpha);
    Result.SkyIntensity = FMath::Lerp(A.SkyIntensity, B.SkyIntensity, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogStartDistance = FMath::Lerp(A.FogStartDistance, B.FogStartDistance, Alpha);
    
    return Result;
}