#include "Light_DayNightCycle.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

ALight_DayNightCycle::ALight_DayNightCycle()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    DayDurationMinutes = 20.0f;
    CurrentTimeOfDay = 0.5f; // Start at noon
    bAutoAdvanceTime = true;
    
    SunLight = nullptr;
    SkyAtmosphere = nullptr;
    HeightFog = nullptr;
}

void ALight_DayNightCycle::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTimeSettings();
    FindLightingActors();
    UpdateLighting();
}

void ALight_DayNightCycle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime && DayDurationMinutes > 0.0f)
    {
        // Advance time based on day duration
        float TimeIncrement = DeltaTime / (DayDurationMinutes * 60.0f);
        CurrentTimeOfDay += TimeIncrement;
        
        // Wrap around at 1.0 (full day cycle)
        if (CurrentTimeOfDay >= 1.0f)
        {
            CurrentTimeOfDay -= 1.0f;
        }
        
        UpdateLighting();
    }
}

void ALight_DayNightCycle::SetTimeOfDay(float NewTime)
{
    CurrentTimeOfDay = FMath::Clamp(NewTime, 0.0f, 1.0f);
    UpdateLighting();
}

ELight_TimeOfDay ALight_DayNightCycle::GetCurrentTimeOfDay() const
{
    if (CurrentTimeOfDay < 0.125f) return ELight_TimeOfDay::Night;
    if (CurrentTimeOfDay < 0.25f) return ELight_TimeOfDay::Dawn;
    if (CurrentTimeOfDay < 0.375f) return ELight_TimeOfDay::Morning;
    if (CurrentTimeOfDay < 0.625f) return ELight_TimeOfDay::Midday;
    if (CurrentTimeOfDay < 0.75f) return ELight_TimeOfDay::Afternoon;
    if (CurrentTimeOfDay < 0.875f) return ELight_TimeOfDay::Sunset;
    return ELight_TimeOfDay::Night;
}

void ALight_DayNightCycle::UpdateLighting()
{
    if (!SunLight || !SkyAtmosphere || !HeightFog)
    {
        FindLightingActors();
        return;
    }
    
    // Get interpolated settings for current time
    FLight_TimeSettings CurrentSettings = InterpolateTimeSettings(CurrentTimeOfDay);
    
    // Update directional light (sun)
    if (UDirectionalLightComponent* LightComp = SunLight->GetLightComponent())
    {
        LightComp->SetIntensity(CurrentSettings.SunIntensity);
        LightComp->SetLightColor(CurrentSettings.SunColor);
        
        // Calculate sun rotation based on time
        float SunAngle = CalculateSunAngle(CurrentTimeOfDay);
        FRotator SunRotation(SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }
    
    // Update sky atmosphere
    if (USkyAtmosphereComponent* SkyComp = SkyAtmosphere->GetComponent<USkyAtmosphereComponent>())
    {
        SkyComp->SetRayleighScatteringScale(CurrentSettings.AtmosphereScale);
    }
    
    // Update height fog
    if (UExponentialHeightFogComponent* FogComp = HeightFog->GetComponent<UExponentialHeightFogComponent>())
    {
        FogComp->SetFogDensity(CurrentSettings.FogDensity);
    }
}

void ALight_DayNightCycle::FindLightingActors()
{
    if (UWorld* World = GetWorld())
    {
        // Find directional light
        TArray<AActor*> FoundLights;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
        if (FoundLights.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundLights[0]);
        }
        
        // Find sky atmosphere
        TArray<AActor*> FoundSky;
        UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundSky);
        if (FoundSky.Num() > 0)
        {
            SkyAtmosphere = Cast<ASkyAtmosphere>(FoundSky[0]);
        }
        
        // Find height fog
        TArray<AActor*> FoundFog;
        UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFog);
        if (FoundFog.Num() > 0)
        {
            HeightFog = Cast<AExponentialHeightFog>(FoundFog[0]);
        }
    }
}

void ALight_DayNightCycle::InitializeTimeSettings()
{
    // Dawn settings
    FLight_TimeSettings DawnSettings;
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.AtmosphereScale = 0.9f;
    TimeSettings.Add(ELight_TimeOfDay::Dawn, DawnSettings);
    
    // Morning settings
    FLight_TimeSettings MorningSettings;
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.AtmosphereScale = 0.8f;
    TimeSettings.Add(ELight_TimeOfDay::Morning, MorningSettings);
    
    // Midday settings
    FLight_TimeSettings MiddaySettings;
    MiddaySettings.SunIntensity = 3.5f;
    MiddaySettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
    MiddaySettings.FogDensity = 0.015f;
    MiddaySettings.AtmosphereScale = 0.7f;
    TimeSettings.Add(ELight_TimeOfDay::Midday, MiddaySettings);
    
    // Afternoon settings
    FLight_TimeSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 2.8f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.7f, 1.0f);
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.AtmosphereScale = 0.75f;
    TimeSettings.Add(ELight_TimeOfDay::Afternoon, AfternoonSettings);
    
    // Sunset settings
    FLight_TimeSettings SunsetSettings;
    SunsetSettings.SunIntensity = 1.8f;
    SunsetSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    SunsetSettings.FogDensity = 0.035f;
    SunsetSettings.AtmosphereScale = 0.85f;
    TimeSettings.Add(ELight_TimeOfDay::Sunset, SunsetSettings);
    
    // Night settings
    FLight_TimeSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.AtmosphereScale = 0.9f;
    TimeSettings.Add(ELight_TimeOfDay::Night, NightSettings);
}

FLight_TimeSettings ALight_DayNightCycle::InterpolateTimeSettings(float TimeValue) const
{
    ELight_TimeOfDay CurrentTime = GetCurrentTimeOfDay();
    
    // Return exact settings if we have them
    if (const FLight_TimeSettings* Settings = TimeSettings.Find(CurrentTime))
    {
        return *Settings;
    }
    
    // Fallback to midday settings
    if (const FLight_TimeSettings* MiddaySettings = TimeSettings.Find(ELight_TimeOfDay::Midday))
    {
        return *MiddaySettings;
    }
    
    // Ultimate fallback
    return FLight_TimeSettings();
}

float ALight_DayNightCycle::CalculateSunAngle(float TimeValue) const
{
    // Convert time (0-1) to sun angle (-90 to 90 degrees)
    // 0.0 (midnight) = -90°, 0.5 (noon) = 90°, 1.0 (midnight) = -90°
    float Angle = FMath::Sin(TimeValue * PI * 2.0f) * 90.0f;
    return Angle;
}