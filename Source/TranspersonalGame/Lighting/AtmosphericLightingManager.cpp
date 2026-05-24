#include "AtmosphericLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

UAtmosphericLightingManager::UAtmosphericLightingManager()
{
    // Initialize default Cretaceous atmospheric settings
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    AtmosphericSettings.SunTemperature = 5500.0f;
    AtmosphericSettings.RayleighScattering = 0.0331f;
    AtmosphericSettings.MieScattering = 0.003996f;
    AtmosphericSettings.MieAbsorption = 0.000444f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    AtmosphericSettings.WhiteTemperature = 6500.0f;
    AtmosphericSettings.ColorSaturation = FVector4(1.2f, 1.2f, 1.2f, 1.0f);

    // Initialize day/night cycle settings
    DayNightSettings.CurrentTimeOfDay = 12.0f; // Start at noon
    DayNightSettings.DayDurationMinutes = 30.0f; // 30 real minutes = 24 game hours
    DayNightSettings.bAutoAdvanceTime = true;

    // Initialize actor references
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    PostProcessVolume = nullptr;
    HeightFog = nullptr;
    
    SunLightComponent = nullptr;
    SkyAtmosphereComponent = nullptr;
    PostProcessComponent = nullptr;
    FogComponent = nullptr;
}

void UAtmosphericLightingManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Initializing..."));
    
    // Find and cache lighting actors
    FindLightingActors();
    CacheLightingComponents();
    
    // Apply Cretaceous defaults
    ResetToCretaceousDefaults();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Initialization complete"));
}

void UAtmosphericLightingManager::Deinitialize()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Deinitializing..."));
    
    // Clear actor references
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    PostProcessVolume = nullptr;
    HeightFog = nullptr;
    
    SunLightComponent = nullptr;
    SkyAtmosphereComponent = nullptr;
    PostProcessComponent = nullptr;
    FogComponent = nullptr;
    
    Super::Deinitialize();
}

bool UAtmosphericLightingManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UAtmosphericLightingManager::Tick(float DeltaTime)
{
    if (DayNightSettings.bAutoAdvanceTime)
    {
        UpdateDayNightCycle(DeltaTime);
    }
    
    if (bIsTransitioningWeather)
    {
        WeatherTransitionTime += DeltaTime;
        if (WeatherTransitionTime >= WeatherTransitionDuration)
        {
            CurrentWeather = TargetWeather;
            bIsTransitioningWeather = false;
            WeatherTransitionTime = 0.0f;
        }
    }
}

void UAtmosphericLightingManager::SetTimeOfDay(float Hours)
{
    DayNightSettings.CurrentTimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (DayNightSettings.CurrentTimeOfDay < 0.0f)
    {
        DayNightSettings.CurrentTimeOfDay += 24.0f;
    }
    
    UpdateSunPosition();
    UpdateSunLighting();
    UpdateSkyAtmosphere();
}

ELight_TimeOfDay UAtmosphericLightingManager::GetCurrentTimeOfDayEnum() const
{
    float Time = DayNightSettings.CurrentTimeOfDay;
    
    if (Time >= 5.0f && Time < 7.0f)
        return ELight_TimeOfDay::Dawn;
    else if (Time >= 7.0f && Time < 11.0f)
        return ELight_TimeOfDay::Morning;
    else if (Time >= 11.0f && Time < 15.0f)
        return ELight_TimeOfDay::Midday;
    else if (Time >= 15.0f && Time < 18.0f)
        return ELight_TimeOfDay::Afternoon;
    else if (Time >= 18.0f && Time < 20.0f)
        return ELight_TimeOfDay::Dusk;
    else
        return ELight_TimeOfDay::Night;
}

void UAtmosphericLightingManager::SetDayDuration(float Minutes)
{
    DayNightSettings.DayDurationMinutes = FMath::Max(1.0f, Minutes);
}

void UAtmosphericLightingManager::PauseTimeAdvancement(bool bPause)
{
    DayNightSettings.bAutoAdvanceTime = !bPause;
}

void UAtmosphericLightingManager::SetWeatherType(ELight_WeatherType NewWeather, float TransitionDuration)
{
    if (NewWeather != CurrentWeather)
    {
        TargetWeather = NewWeather;
        WeatherTransitionDuration = FMath::Max(0.1f, TransitionDuration);
        WeatherTransitionTime = 0.0f;
        bIsTransitioningWeather = true;
    }
}

void UAtmosphericLightingManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    AtmosphericSettings = Settings;
    
    UpdateSunLighting();
    UpdateSkyAtmosphere();
    UpdatePostProcessing();
    UpdateFog();
}

void UAtmosphericLightingManager::ResetToCretaceousDefaults()
{
    UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Applying Cretaceous defaults"));
    
    // Reset to default Cretaceous settings
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    AtmosphericSettings.SunTemperature = 5500.0f;
    AtmosphericSettings.RayleighScattering = 0.0331f;
    AtmosphericSettings.MieScattering = 0.003996f;
    AtmosphericSettings.MieAbsorption = 0.000444f;
    
    ApplyAtmosphericSettings(AtmosphericSettings);
    SetTimeOfDay(12.0f); // Reset to noon
    SetWeatherType(ELight_WeatherType::Clear, 0.0f);
}

void UAtmosphericLightingManager::RefreshLightingActors()
{
    FindLightingActors();
    CacheLightingComponents();
    ApplyAtmosphericSettings(AtmosphericSettings);
}

void UAtmosphericLightingManager::ApplyCretaceousLighting()
{
    ResetToCretaceousDefaults();
}

void UAtmosphericLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    // Convert real time to game time
    float GameHoursPerRealSecond = 24.0f / (DayNightSettings.DayDurationMinutes * 60.0f);
    float TimeIncrement = DeltaTime * GameHoursPerRealSecond;
    
    DayNightSettings.CurrentTimeOfDay += TimeIncrement;
    if (DayNightSettings.CurrentTimeOfDay >= 24.0f)
    {
        DayNightSettings.CurrentTimeOfDay -= 24.0f;
    }
    
    UpdateSunPosition();
    UpdateSunLighting();
    UpdateSkyAtmosphere();
}

void UAtmosphericLightingManager::UpdateSunPosition()
{
    if (!SunLight || !SunLightComponent)
        return;
    
    // Calculate sun angle based on time of day
    float SunAngle = (DayNightSettings.CurrentTimeOfDay - 6.0f) * 15.0f; // 15 degrees per hour, sunrise at 6 AM
    
    // Create rotation for sun direction
    FRotator SunRotation = FRotator(-SunAngle, 45.0f, 0.0f); // Pitch based on time, fixed yaw
    SunLight->SetActorRotation(SunRotation);
}

void UAtmosphericLightingManager::UpdateSunLighting()
{
    if (!SunLightComponent)
        return;
    
    float Time = DayNightSettings.CurrentTimeOfDay;
    float IntensityMultiplier = 1.0f;
    FLinearColor ColorMultiplier = FLinearColor::White;
    
    // Calculate intensity based on time of day
    if (Time >= 6.0f && Time <= 18.0f) // Daytime
    {
        float NoonFactor = 1.0f - FMath::Abs(Time - 12.0f) / 6.0f;
        IntensityMultiplier = FMath::Lerp(0.1f, 1.0f, NoonFactor);
        
        // Warmer colors during sunrise/sunset
        if (Time < 8.0f || Time > 16.0f)
        {
            float WarmFactor = (Time < 8.0f) ? (8.0f - Time) / 2.0f : (Time - 16.0f) / 2.0f;
            ColorMultiplier = FLinearColor::LerpUsingHSV(FLinearColor::White, FLinearColor(1.0f, 0.7f, 0.4f), WarmFactor);
        }
    }
    else // Nighttime
    {
        IntensityMultiplier = 0.05f; // Moonlight
        ColorMultiplier = FLinearColor(0.7f, 0.8f, 1.0f); // Cool blue moonlight
    }
    
    // Apply settings
    SunLightComponent->SetIntensity(AtmosphericSettings.SunIntensity * IntensityMultiplier);
    SunLightComponent->SetLightColor(AtmosphericSettings.SunColor * ColorMultiplier);
    SunLightComponent->SetTemperature(AtmosphericSettings.SunTemperature);
}

void UAtmosphericLightingManager::UpdateSkyAtmosphere()
{
    if (!SkyAtmosphereComponent)
        return;
    
    SkyAtmosphereComponent->SetRayleighScatteringScale(AtmosphericSettings.RayleighScattering);
    SkyAtmosphereComponent->SetMieScatteringScale(AtmosphericSettings.MieScattering);
    SkyAtmosphereComponent->SetMieAbsorptionScale(AtmosphericSettings.MieAbsorption);
}

void UAtmosphericLightingManager::UpdatePostProcessing()
{
    if (!PostProcessComponent)
        return;
    
    FPostProcessSettings& Settings = PostProcessComponent->Settings;
    Settings.bOverride_WhiteTemp = true;
    Settings.WhiteTemp = AtmosphericSettings.WhiteTemperature;
    Settings.bOverride_ColorSaturation = true;
    Settings.ColorSaturation = AtmosphericSettings.ColorSaturation;
}

void UAtmosphericLightingManager::UpdateFog()
{
    if (!FogComponent)
        return;
    
    FogComponent->SetFogDensity(AtmosphericSettings.FogDensity);
    FogComponent->SetFogInscatteringColor(AtmosphericSettings.FogColor);
}

void UAtmosphericLightingManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find DirectionalLight
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Found DirectionalLight"));
    }
    
    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Found SkyAtmosphere"));
    }
    
    // Find PostProcessVolume
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        PostProcessVolume = Cast<APostProcessVolume>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Found PostProcessVolume"));
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
        UE_LOG(LogTemp, Log, TEXT("AtmosphericLightingManager: Found ExponentialHeightFog"));
    }
}

void UAtmosphericLightingManager::CacheLightingComponents()
{
    // Cache DirectionalLight component
    if (SunLight)
    {
        SunLightComponent = SunLight->GetLightComponent();
    }
    
    // Cache SkyAtmosphere component
    if (SkyAtmosphere)
    {
        SkyAtmosphereComponent = SkyAtmosphere->GetComponent();
    }
    
    // Cache PostProcess component
    if (PostProcessVolume)
    {
        PostProcessComponent = PostProcessVolume->GetComponent();
    }
    
    // Cache Fog component
    if (HeightFog)
    {
        FogComponent = HeightFog->GetComponent();
    }
}