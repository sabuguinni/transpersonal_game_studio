#include "Light_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphereManager::ALight_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default atmospheric settings for Cretaceous period
    // Dawn - soft pink/orange light
    DawnSettings.SunIntensity = 3.0f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.SunTemperature = 3000.0f;
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.SkyLightIntensity = 0.8f;
    
    // Morning - warm golden light
    MorningSettings.SunIntensity = 6.0f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunTemperature = 4500.0f;
    MorningSettings.FogDensity = 0.025f;
    MorningSettings.SkyLightIntensity = 1.0f;
    
    // Midday - bright white/yellow light (default Cretaceous)
    MiddaySettings.SunIntensity = 8.0f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    MiddaySettings.SunTemperature = 5800.0f;
    MiddaySettings.FogDensity = 0.02f;
    MiddaySettings.SkyLightIntensity = 1.2f;
    
    // Afternoon - warm amber light
    AfternoonSettings.SunIntensity = 7.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    AfternoonSettings.SunTemperature = 4000.0f;
    AfternoonSettings.FogDensity = 0.03f;
    AfternoonSettings.SkyLightIntensity = 1.1f;
    
    // Dusk - deep orange/red light
    DuskSettings.SunIntensity = 4.0f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunTemperature = 2500.0f;
    DuskSettings.FogDensity = 0.05f;
    DuskSettings.SkyLightIntensity = 0.6f;
    
    // Night - minimal moonlight
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    NightSettings.SunTemperature = 8000.0f;
    NightSettings.FogDensity = 0.06f;
    NightSettings.SkyLightIntensity = 0.3f;
}

void ALight_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingActors();
    EnableLumenGlobalIllumination();
    
    // Apply current time of day settings
    SetTimeOfDay(CurrentTimeOfDay);
}

void ALight_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphereManager::InitializeLightingActors()
{
    if (bLightingActorsInitialized)
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Find existing lighting actors
    TArray<AActor*> FoundActors;
    
    // Find DirectionalLight
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunActor = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    // Find SkyLight
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLightActor = Cast<ASkyLight>(FoundActors[0]);
    }
    
    // Find ExponentialHeightFog
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        FogActor = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    // Find SkyAtmosphere
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphereActor = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    bLightingActorsInitialized = true;
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Initialized lighting actors - Sun:%s, Sky:%s, Fog:%s, Atmosphere:%s"),
        SunActor ? TEXT("Found") : TEXT("Missing"),
        SkyLightActor ? TEXT("Found") : TEXT("Missing"),
        FogActor ? TEXT("Found") : TEXT("Missing"),
        SkyAtmosphereActor ? TEXT("Found") : TEXT("Missing"));
}

void ALight_AtmosphereManager::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    FLight_AtmosphericSettings TargetSettings;
    
    switch (NewTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            TargetSettings = DawnSettings;
            CurrentTimeOfDayFloat = 0.25f;
            break;
        case ELight_TimeOfDay::Morning:
            TargetSettings = MorningSettings;
            CurrentTimeOfDayFloat = 0.375f;
            break;
        case ELight_TimeOfDay::Midday:
            TargetSettings = MiddaySettings;
            CurrentTimeOfDayFloat = 0.5f;
            break;
        case ELight_TimeOfDay::Afternoon:
            TargetSettings = AfternoonSettings;
            CurrentTimeOfDayFloat = 0.625f;
            break;
        case ELight_TimeOfDay::Dusk:
            TargetSettings = DuskSettings;
            CurrentTimeOfDayFloat = 0.75f;
            break;
        case ELight_TimeOfDay::Night:
            TargetSettings = NightSettings;
            CurrentTimeOfDayFloat = 0.0f;
            break;
    }
    
    ApplyWeatherModifications(TargetSettings);
    ApplyAtmosphericSettings(TargetSettings);
    UpdateSunPosition();
}

void ALight_AtmosphereManager::SetWeather(ELight_WeatherType NewWeather)
{
    CurrentWeather = NewWeather;
    
    // Reapply current time settings with new weather modifications
    SetTimeOfDay(CurrentTimeOfDay);
}

void ALight_AtmosphereManager::SetTimeOfDayFloat(float TimeFloat)
{
    CurrentTimeOfDayFloat = FMath::Clamp(TimeFloat, 0.0f, 1.0f);
    
    // Interpolate between time periods
    FLight_AtmosphericSettings InterpolatedSettings;
    
    if (TimeFloat < 0.25f) // Night to Dawn
    {
        float Alpha = TimeFloat / 0.25f;
        InterpolatedSettings = InterpolateSettings(NightSettings, DawnSettings, Alpha);
    }
    else if (TimeFloat < 0.375f) // Dawn to Morning
    {
        float Alpha = (TimeFloat - 0.25f) / 0.125f;
        InterpolatedSettings = InterpolateSettings(DawnSettings, MorningSettings, Alpha);
    }
    else if (TimeFloat < 0.5f) // Morning to Midday
    {
        float Alpha = (TimeFloat - 0.375f) / 0.125f;
        InterpolatedSettings = InterpolateSettings(MorningSettings, MiddaySettings, Alpha);
    }
    else if (TimeFloat < 0.625f) // Midday to Afternoon
    {
        float Alpha = (TimeFloat - 0.5f) / 0.125f;
        InterpolatedSettings = InterpolateSettings(MiddaySettings, AfternoonSettings, Alpha);
    }
    else if (TimeFloat < 0.75f) // Afternoon to Dusk
    {
        float Alpha = (TimeFloat - 0.625f) / 0.125f;
        InterpolatedSettings = InterpolateSettings(AfternoonSettings, DuskSettings, Alpha);
    }
    else // Dusk to Night
    {
        float Alpha = (TimeFloat - 0.75f) / 0.25f;
        InterpolatedSettings = InterpolateSettings(DuskSettings, NightSettings, Alpha);
    }
    
    ApplyWeatherModifications(InterpolatedSettings);
    ApplyAtmosphericSettings(InterpolatedSettings);
    UpdateSunPosition();
}

void ALight_AtmosphereManager::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    if (!bLightingActorsInitialized)
    {
        InitializeLightingActors();
    }
    
    // Apply sun settings
    if (SunActor && SunActor->GetLightComponent())
    {
        UDirectionalLightComponent* SunComponent = SunActor->GetLightComponent();
        SunComponent->SetIntensity(Settings.SunIntensity);
        SunComponent->SetLightColor(Settings.SunColor);
        SunComponent->SetTemperature(Settings.SunTemperature);
        SunComponent->SetVolumetricScatteringIntensity(Settings.VolumetricScatteringIntensity);
        SunComponent->SetCastVolumetricShadow(true);
    }
    
    // Apply sky light settings
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComponent = SkyLightActor->GetLightComponent();
        SkyComponent->SetIntensity(Settings.SkyLightIntensity);
    }
    
    // Apply fog settings
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = FogActor->GetComponent();
        FogComponent->SetFogDensity(Settings.FogDensity);
        FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComponent->SetFogInscatteringColor(Settings.FogInscatteringColor);
        FogComponent->SetVolumetricFog(true);
        FogComponent->SetVolumetricFogScatteringDistribution(0.6f);
    }
}

void ALight_AtmosphereManager::EnableLumenGlobalIllumination()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    // Enable Lumen via console commands
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.DynamicGlobalIllumination.Method 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.GlobalIllumination 1"));
    UKismetSystemLibrary::ExecuteConsoleCommand(World, TEXT("r.Lumen.Reflections 1"));
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphereManager: Enabled Lumen Global Illumination"));
}

void ALight_AtmosphereManager::UpdateSunPosition()
{
    if (!SunActor)
        return;
    
    float SunAngle = CalculateSunAngle();
    
    // Create sun rotation based on time of day
    FRotator SunRotation;
    SunRotation.Pitch = SunAngle;
    SunRotation.Yaw = 0.0f; // Sun moves from east to west
    SunRotation.Roll = 0.0f;
    
    SunActor->SetActorRotation(SunRotation);
}

FLight_AtmosphericSettings ALight_AtmosphereManager::GetCurrentAtmosphericSettings() const
{
    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn: return DawnSettings;
        case ELight_TimeOfDay::Morning: return MorningSettings;
        case ELight_TimeOfDay::Midday: return MiddaySettings;
        case ELight_TimeOfDay::Afternoon: return AfternoonSettings;
        case ELight_TimeOfDay::Dusk: return DuskSettings;
        case ELight_TimeOfDay::Night: return NightSettings;
        default: return MiddaySettings;
    }
}

void ALight_AtmosphereManager::UpdateDayNightCycle(float DeltaTime)
{
    AccumulatedTime += DeltaTime;
    
    float CycleDurationSeconds = DayDurationMinutes * 60.0f;
    float CycleProgress = FMath::Fmod(AccumulatedTime, CycleDurationSeconds) / CycleDurationSeconds;
    
    SetTimeOfDayFloat(CycleProgress);
}

FLight_AtmosphericSettings ALight_AtmosphereManager::InterpolateSettings(const FLight_AtmosphericSettings& A, const FLight_AtmosphericSettings& B, float Alpha) const
{
    FLight_AtmosphericSettings Result;
    
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunColor = FMath::Lerp(A.SunColor, B.SunColor, Alpha);
    Result.SunTemperature = FMath::Lerp(A.SunTemperature, B.SunTemperature, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    Result.FogInscatteringColor = FMath::Lerp(A.FogInscatteringColor, B.FogInscatteringColor, Alpha);
    Result.VolumetricScatteringIntensity = FMath::Lerp(A.VolumetricScatteringIntensity, B.VolumetricScatteringIntensity, Alpha);
    Result.SkyLightIntensity = FMath::Lerp(A.SkyLightIntensity, B.SkyLightIntensity, Alpha);
    
    return Result;
}

void ALight_AtmosphereManager::ApplyWeatherModifications(FLight_AtmosphericSettings& Settings) const
{
    switch (CurrentWeather)
    {
        case ELight_WeatherType::Hazy:
            Settings.FogDensity *= HazyFogMultiplier;
            Settings.VolumetricScatteringIntensity *= 1.5f;
            break;
            
        case ELight_WeatherType::Overcast:
            Settings.SunIntensity *= OvercastSunIntensityMultiplier;
            Settings.SkyLightIntensity *= 1.5f;
            Settings.FogDensity *= 1.5f;
            break;
            
        case ELight_WeatherType::Stormy:
            Settings.SunIntensity *= 0.2f;
            Settings.FogDensity *= StormyFogMultiplier;
            Settings.SkyLightIntensity *= 0.5f;
            Settings.FogInscatteringColor = FLinearColor(0.4f, 0.4f, 0.5f, 1.0f);
            break;
            
        case ELight_WeatherType::Foggy:
            Settings.FogDensity *= FoggyVisibilityMultiplier;
            Settings.SunIntensity *= 0.3f;
            Settings.VolumetricScatteringIntensity *= 3.0f;
            break;
            
        case ELight_WeatherType::Clear:
        default:
            // No modifications for clear weather
            break;
    }
}

float ALight_AtmosphereManager::CalculateSunAngle() const
{
    // Convert time of day to sun angle
    // 0.0 (midnight) = -90 degrees (below horizon)
    // 0.5 (noon) = 90 degrees (overhead)
    // 1.0 (midnight) = -90 degrees (below horizon)
    
    float SunAngle;
    if (CurrentTimeOfDayFloat <= 0.5f)
    {
        // Rising sun: -90 to +90 degrees
        SunAngle = FMath::Lerp(-90.0f, 90.0f, CurrentTimeOfDayFloat * 2.0f);
    }
    else
    {
        // Setting sun: +90 to -90 degrees
        SunAngle = FMath::Lerp(90.0f, -90.0f, (CurrentTimeOfDayFloat - 0.5f) * 2.0f);
    }
    
    return SunAngle;
}