#include "LightingSystemManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"

void ULightingSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Initializing"));
    
    // Find existing lighting actors in the world
    FindLightingActors();
    
    // Apply default Cretaceous lighting
    ApplyCretaceousLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Initialization complete"));
}

void ULightingSystemManager::Deinitialize()
{
    bDayNightCycleEnabled = false;
    SunActor = nullptr;
    SkyAtmosphereActor = nullptr;
    VolumetricCloudActor = nullptr;
    FogActor = nullptr;
    
    Super::Deinitialize();
}

void ULightingSystemManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("LightingSystemManager: No valid world"));
        return;
    }

    // Find DirectionalLight (Sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(DirectionalLights[0]);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found DirectionalLight"));
    }

    // Find SkyAtmosphere
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("SkyAtmosphere")))
        {
            SkyAtmosphereActor = Actor;
            UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found SkyAtmosphere"));
            break;
        }
    }

    // Find VolumetricCloud
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("VolumetricCloud")))
        {
            VolumetricCloudActor = Actor;
            UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found VolumetricCloud"));
            break;
        }
    }

    // Find ExponentialHeightFog
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetClass()->GetName().Contains(TEXT("ExponentialHeightFog")))
        {
            FogActor = Actor;
            UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Found ExponentialHeightFog"));
            break;
        }
    }
}

void ULightingSystemManager::SetTimeOfDay(float Hours)
{
    TimeSettings.CurrentTimeHours = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateSunPosition();
    UpdateSunLighting();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Time set to %f hours"), Hours);
}

void ULightingSystemManager::SetDayDuration(float Minutes)
{
    TimeSettings.DayDurationMinutes = FMath::Max(Minutes, 1.0f);
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Day duration set to %f minutes"), Minutes);
}

void ULightingSystemManager::EnableDayNightCycle(bool bEnable)
{
    bDayNightCycleEnabled = bEnable;
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Day/Night cycle %s"), 
           bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void ULightingSystemManager::SetWeather(EWeatherType NewWeather)
{
    WeatherSettings.CurrentWeather = NewWeather;
    UpdateWeatherEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Weather changed"));
}

void ULightingSystemManager::SetSunIntensity(float Intensity)
{
    if (SunActor && SunActor->GetLightComponent())
    {
        SunActor->GetLightComponent()->SetIntensity(Intensity);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Sun intensity set to %f"), Intensity);
    }
}

void ULightingSystemManager::SetSunColor(FLinearColor Color)
{
    if (SunActor && SunActor->GetLightComponent())
    {
        SunActor->GetLightComponent()->SetLightColor(Color);
        UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Sun color updated"));
    }
}

void ULightingSystemManager::SetAtmosphereSettings(float ScatteringScale)
{
    TimeSettings.AtmosphereScattering = ScatteringScale;
    UpdateAtmosphere();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Atmosphere scattering set to %f"), ScatteringScale);
}

void ULightingSystemManager::SetFogSettings(float Density, FLinearColor Color)
{
    TimeSettings.FogDensity = Density;
    TimeSettings.FogColor = Color;
    
    if (FogActor)
    {
        UExponentialHeightFogComponent* FogComponent = FogActor->FindComponentByClass<UExponentialHeightFogComponent>();
        if (FogComponent)
        {
            FogComponent->SetFogDensity(Density);
            FogComponent->SetFogInscatteringColor(Color);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Fog settings updated"));
}

void ULightingSystemManager::ApplyCretaceousLighting()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applying Cretaceous period lighting"));
    
    // Set tropical daytime lighting
    TimeSettings.CurrentTimeHours = 12.0f; // Noon
    TimeSettings.SunIntensityDay = 5.0f;
    TimeSettings.SunColorDay = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f); // Warm white
    TimeSettings.AtmosphereScattering = 0.0331f; // Clear blue sky
    
    // Disable clouds for clear visibility
    WeatherSettings.bCloudsEnabled = false;
    WeatherSettings.CurrentWeather = EWeatherType::Clear;
    
    // Apply settings
    ApplyLightingSettings();
}

void ULightingSystemManager::ApplyTropicalAtmosphere()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applying tropical atmosphere"));
    
    // Tropical atmosphere settings
    TimeSettings.AtmosphereScattering = 0.0331f;
    TimeSettings.FogDensity = 0.02f;
    TimeSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    UpdateAtmosphere();
    SetFogSettings(TimeSettings.FogDensity, TimeSettings.FogColor);
}

void ULightingSystemManager::ApplyJungleLighting()
{
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: Applying jungle lighting"));
    
    // Jungle lighting with filtered sunlight
    TimeSettings.SunIntensityDay = 4.0f; // Slightly dimmed by canopy
    TimeSettings.SunColorDay = FLinearColor(0.9f, 1.0f, 0.8f, 1.0f); // Green tint
    TimeSettings.FogDensity = 0.03f; // More atmospheric
    
    ApplyLightingSettings();
}

void ULightingSystemManager::Tick(float DeltaTime)
{
    if (!bDayNightCycleEnabled)
        return;

    // Update time accumulator
    TimeAccumulator += DeltaTime;
    
    // Calculate time progression (real seconds to game hours)
    float SecondsPerGameHour = (TimeSettings.DayDurationMinutes * 60.0f) / 24.0f;
    float HoursDelta = DeltaTime / SecondsPerGameHour;
    
    // Update current time
    TimeSettings.CurrentTimeHours += HoursDelta;
    if (TimeSettings.CurrentTimeHours >= 24.0f)
    {
        TimeSettings.CurrentTimeHours -= 24.0f;
    }
    
    // Update lighting every few seconds to avoid performance issues
    if (TimeAccumulator >= 2.0f)
    {
        UpdateSunPosition();
        UpdateSunLighting();
        TimeAccumulator = 0.0f;
    }
}

TStatId ULightingSystemManager::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(ULightingSystemManager, STATGROUP_Tickables);
}

void ULightingSystemManager::UpdateSunPosition()
{
    if (!SunActor)
        return;

    // Calculate sun angle based on time (0-24 hours)
    float SunAngle = (TimeSettings.CurrentTimeHours / 24.0f) * 360.0f - 90.0f; // -90 to start at sunrise
    
    // Convert to radians and create rotation
    float SunAngleRad = FMath::DegreesToRadians(SunAngle);
    FRotator SunRotation = FRotator(-SunAngle, 0.0f, 0.0f); // Pitch rotation for sun arc
    
    SunActor->SetActorRotation(SunRotation);
}

void ULightingSystemManager::UpdateSunLighting()
{
    if (!SunActor || !SunActor->GetLightComponent())
        return;

    // Calculate sun intensity based on time (day/night cycle)
    float SunHeight = FMath::Sin((TimeSettings.CurrentTimeHours / 24.0f) * 2.0f * PI);
    float IntensityLerp = FMath::Clamp(SunHeight, 0.0f, 1.0f);
    
    float CurrentIntensity = FMath::Lerp(TimeSettings.SunIntensityNight, TimeSettings.SunIntensityDay, IntensityLerp);
    FLinearColor CurrentColor = FMath::Lerp(TimeSettings.SunColorNight, TimeSettings.SunColorDay, IntensityLerp);
    
    SunActor->GetLightComponent()->SetIntensity(CurrentIntensity);
    SunActor->GetLightComponent()->SetLightColor(CurrentColor);
}

void ULightingSystemManager::UpdateAtmosphere()
{
    if (!SkyAtmosphereActor)
        return;

    USkyAtmosphereComponent* AtmosphereComponent = SkyAtmosphereActor->FindComponentByClass<USkyAtmosphereComponent>();
    if (AtmosphereComponent)
    {
        AtmosphereComponent->SetRayleighScatteringScale(TimeSettings.AtmosphereScattering);
    }
}

void ULightingSystemManager::UpdateWeatherEffects()
{
    // Hide/show volumetric clouds based on weather
    if (VolumetricCloudActor)
    {
        bool bShowClouds = (WeatherSettings.CurrentWeather != EWeatherType::Clear) && WeatherSettings.bCloudsEnabled;
        VolumetricCloudActor->SetActorHiddenInGame(!bShowClouds);
    }
    
    // Update fog based on weather
    if (WeatherSettings.CurrentWeather == EWeatherType::Foggy)
    {
        SetFogSettings(0.1f, FLinearColor(0.7f, 0.7f, 0.8f, 1.0f));
    }
    else
    {
        SetFogSettings(TimeSettings.FogDensity, TimeSettings.FogColor);
    }
}

void ULightingSystemManager::ApplyLightingSettings()
{
    // Apply all current settings to actors
    SetSunIntensity(TimeSettings.SunIntensityDay);
    SetSunColor(TimeSettings.SunColorDay);
    UpdateSunPosition();
    UpdateAtmosphere();
    SetFogSettings(TimeSettings.FogDensity, TimeSettings.FogColor);
    UpdateWeatherEffects();
    
    UE_LOG(LogTemp, Warning, TEXT("LightingSystemManager: All lighting settings applied"));
}