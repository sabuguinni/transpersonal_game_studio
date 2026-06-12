#include "Light_AtmosphereManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/PostProcessComponent.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize components
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    
    SunLightComponent = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLightComponent"));
    SunLightComponent->SetupAttachment(RootComponent);
    
    SkyAtmosphereComponent = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphereComponent"));
    SkyAtmosphereComponent->SetupAttachment(RootComponent);
    
    HeightFogComponent = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFogComponent"));
    HeightFogComponent->SetupAttachment(RootComponent);
    
    // Set default Cretaceous period lighting
    ApplyCretaceousLighting();
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeComponents();
    ApplyCretaceousLighting();
    
    // Find or create PostProcessVolume
    TArray<AActor*> PostProcessActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APostProcessVolume::StaticClass(), PostProcessActors);
    
    if (PostProcessActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(PostProcessActors[0]);
    }
    else
    {
        // Create PostProcessVolume if none exists
        PostProcessVolume = GetWorld()->SpawnActor<APostProcessVolume>();
        if (PostProcessVolume)
        {
            PostProcessVolume->bUnbound = true;
            // Enable Lumen
            PostProcessVolume->Settings.bOverride_GlobalIlluminationMethod = true;
            PostProcessVolume->Settings.GlobalIlluminationMethod = EGlobalIlluminationMethod::Lumen;
            PostProcessVolume->Settings.bOverride_ReflectionMethod = true;
            PostProcessVolume->Settings.ReflectionMethod = EReflectionMethod::Lumen;
        }
    }
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        TimeAccumulator += DeltaTime;
        
        // Convert real time to game time
        float GameTimeIncrement = (24.0f / (DayDurationMinutes * 60.0f)) * DeltaTime;
        CurrentTimeOfDay += GameTimeIncrement;
        
        if (CurrentTimeOfDay >= 24.0f)
        {
            CurrentTimeOfDay -= 24.0f;
        }
        
        UpdateLightingBasedOnTime();
        UpdateSunPosition();
    }
    
    UpdateWeatherEffects();
}

void ALight_AtmosphereManager::InitializeComponents()
{
    if (SunLightComponent)
    {
        SunLightComponent->SetIntensity(CretaceousSettings.SunIntensity);
        SunLightComponent->SetLightColor(CretaceousSettings.SunColor);
        SunLightComponent->SetTemperature(CretaceousSettings.Temperature);
        SunLightComponent->SetCastShadows(true);
        SunLightComponent->SetCastVolumetricShadow(true);
    }
    
    if (HeightFogComponent)
    {
        HeightFogComponent->SetFogDensity(CretaceousSettings.FogDensity);
        HeightFogComponent->SetFogHeightFalloff(CretaceousSettings.FogHeightFalloff);
        HeightFogComponent->SetStartDistance(CretaceousSettings.StartDistance);
        HeightFogComponent->SetVolumetricFog(CretaceousSettings.bVolumetricFog);
        HeightFogComponent->SetVolumetricFogScatteringDistribution(0.2f);
        HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
    }
}

void ALight_AtmosphereManager::SetTimeOfDay(float Hours)
{
    CurrentTimeOfDay = FMath::Clamp(Hours, 0.0f, 24.0f);
    UpdateLightingBasedOnTime();
    UpdateSunPosition();
}

void ALight_AtmosphereManager::SetWeather(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    UpdateWeatherEffects();
}

void ALight_AtmosphereManager::ApplyCretaceousLighting()
{
    if (SunLightComponent)
    {
        // Cretaceous period had higher CO2, creating warmer, more humid atmosphere
        // Sunlight would appear more golden/amber due to atmospheric conditions
        SunLightComponent->SetIntensity(5.0f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.94f, 0.78f, 1.0f)); // Warm golden
        SunLightComponent->SetTemperature(5500.0f); // Slightly warmer than modern sun
        SunLightComponent->SetCastShadows(true);
        SunLightComponent->SetCastVolumetricShadow(true);
    }
    
    if (HeightFogComponent)
    {
        // Higher humidity in Cretaceous atmosphere
        HeightFogComponent->SetFogDensity(0.025f); // Slightly denser
        HeightFogComponent->SetFogHeightFalloff(0.15f); // More gradual falloff
        HeightFogComponent->SetStartDistance(800.0f); // Closer fog start
        HeightFogComponent->SetVolumetricFog(true);
        HeightFogComponent->SetVolumetricFogScatteringDistribution(0.3f);
        HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.92f, 0.85f)); // Warm fog tint
    }
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (SunLightComponent)
    {
        // Calculate sun angle based on time of day
        float SunAngle = GetSunAngle();
        
        // Convert to rotation (sun moves from east to west)
        FRotator SunRotation;
        SunRotation.Pitch = SunAngle;
        SunRotation.Yaw = (CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
        SunRotation.Roll = 0.0f;
        
        SunLightComponent->SetWorldRotation(SunRotation);
    }
}

void ALight_AtmosphereManager::UpdateAtmosphericFog()
{
    if (HeightFogComponent)
    {
        ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
        
        switch (TimeEnum)
        {
        case ELight_TimeOfDay::Dawn:
            HeightFogComponent->SetFogDensity(0.04f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(1.0f, 0.8f, 0.6f));
            break;
        case ELight_TimeOfDay::Morning:
            HeightFogComponent->SetFogDensity(0.02f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.92f, 0.85f));
            break;
        case ELight_TimeOfDay::Midday:
            HeightFogComponent->SetFogDensity(0.015f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.9f));
            break;
        case ELight_TimeOfDay::Afternoon:
            HeightFogComponent->SetFogDensity(0.02f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.95f, 0.9f, 0.8f));
            break;
        case ELight_TimeOfDay::Dusk:
            HeightFogComponent->SetFogDensity(0.035f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(1.0f, 0.7f, 0.5f));
            break;
        case ELight_TimeOfDay::Night:
            HeightFogComponent->SetFogDensity(0.03f);
            HeightFogComponent->SetVolumetricFogAlbedo(FLinearColor(0.3f, 0.4f, 0.6f));
            break;
        }
    }
}

ELight_TimeOfDay ALight_AtmosphereManager::GetCurrentTimeOfDayEnum() const
{
    if (CurrentTimeOfDay >= 5.0f && CurrentTimeOfDay < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (CurrentTimeOfDay >= 7.0f && CurrentTimeOfDay < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (CurrentTimeOfDay >= 11.0f && CurrentTimeOfDay < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (CurrentTimeOfDay >= 15.0f && CurrentTimeOfDay < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (CurrentTimeOfDay >= 18.0f && CurrentTimeOfDay < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

float ALight_AtmosphereManager::GetSunAngle() const
{
    // Sun angle calculation: -90 (sunrise) to +90 (sunset)
    // Peak at 0 degrees (noon)
    float NormalizedTime = (CurrentTimeOfDay - 6.0f) / 12.0f; // 0 at sunrise, 1 at sunset
    float SunAngle = FMath::Sin(NormalizedTime * PI) * 90.0f - 90.0f;
    return FMath::Clamp(SunAngle, -90.0f, 90.0f);
}

void ALight_AtmosphereManager::UpdateLightingBasedOnTime()
{
    if (!SunLightComponent)
        return;
    
    ELight_TimeOfDay TimeEnum = GetCurrentTimeOfDayEnum();
    
    switch (TimeEnum)
    {
    case ELight_TimeOfDay::Dawn:
        SunLightComponent->SetIntensity(2.0f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.7f, 0.4f, 1.0f)); // Orange dawn
        SunLightComponent->SetTemperature(3000.0f);
        break;
    case ELight_TimeOfDay::Morning:
        SunLightComponent->SetIntensity(4.0f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f)); // Warm morning
        SunLightComponent->SetTemperature(4500.0f);
        break;
    case ELight_TimeOfDay::Midday:
        SunLightComponent->SetIntensity(6.0f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.95f, 0.85f, 1.0f)); // Bright midday
        SunLightComponent->SetTemperature(5800.0f);
        break;
    case ELight_TimeOfDay::Afternoon:
        SunLightComponent->SetIntensity(4.5f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.9f, 0.75f, 1.0f)); // Warm afternoon
        SunLightComponent->SetTemperature(5000.0f);
        break;
    case ELight_TimeOfDay::Dusk:
        SunLightComponent->SetIntensity(1.5f);
        SunLightComponent->SetLightColor(FLinearColor(1.0f, 0.6f, 0.3f, 1.0f)); // Red sunset
        SunLightComponent->SetTemperature(2500.0f);
        break;
    case ELight_TimeOfDay::Night:
        SunLightComponent->SetIntensity(0.1f);
        SunLightComponent->SetLightColor(FLinearColor(0.3f, 0.4f, 0.8f, 1.0f)); // Moonlight blue
        SunLightComponent->SetTemperature(4000.0f);
        break;
    }
    
    UpdateAtmosphericFog();
}

void ALight_AtmosphereManager::UpdateWeatherEffects()
{
    if (!HeightFogComponent)
        return;
    
    switch (CurrentWeather)
    {
    case ELight_WeatherType::Clear:
        HeightFogComponent->SetFogDensity(CretaceousSettings.FogDensity);
        if (SunLightComponent)
            SunLightComponent->SetIntensity(SunLightComponent->Intensity * 1.0f);
        break;
    case ELight_WeatherType::Overcast:
        HeightFogComponent->SetFogDensity(CretaceousSettings.FogDensity * 1.5f);
        if (SunLightComponent)
            SunLightComponent->SetIntensity(SunLightComponent->Intensity * 0.6f);
        break;
    case ELight_WeatherType::Foggy:
        HeightFogComponent->SetFogDensity(CretaceousSettings.FogDensity * 3.0f);
        if (SunLightComponent)
            SunLightComponent->SetIntensity(SunLightComponent->Intensity * 0.3f);
        break;
    case ELight_WeatherType::Storm:
        HeightFogComponent->SetFogDensity(CretaceousSettings.FogDensity * 2.0f);
        if (SunLightComponent)
            SunLightComponent->SetIntensity(SunLightComponent->Intensity * 0.4f);
        break;
    }
}