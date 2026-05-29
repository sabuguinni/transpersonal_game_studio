#include "AtmosphericManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AAtmosphericManager::AAtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize lighting presets
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f);
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunRotation = FRotator(-5.0f, 75.0f, 0.0f);
    DawnSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.4f);
    DawnSettings.FogDensity = 0.03f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);

    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.8f);
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunRotation = FRotator(-25.0f, 105.0f, 0.0f);
    MorningSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f);
    MorningSettings.FogDensity = 0.02f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);

    FEnvArt_LightingSettings NoonSettings;
    NoonSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f);
    NoonSettings.SunIntensity = 4.0f;
    NoonSettings.SunRotation = FRotator(-75.0f, 180.0f, 0.0f);
    NoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    NoonSettings.FogDensity = 0.015f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Noon, NoonSettings);

    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.86f, 0.69f);
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunRotation = FRotator(-15.0f, 225.0f, 0.0f);
    AfternoonSettings.FogColor = FLinearColor(0.9f, 0.7f, 0.4f);
    AfternoonSettings.FogDensity = 0.02f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);

    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.5f, 0.2f);
    DuskSettings.SunIntensity = 1.0f;
    DuskSettings.SunRotation = FRotator(5.0f, 285.0f, 0.0f);
    DuskSettings.FogColor = FLinearColor(0.8f, 0.4f, 0.2f);
    DuskSettings.FogDensity = 0.04f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);

    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunRotation = FRotator(15.0f, 0.0f, 0.0f);
    NightSettings.FogColor = FLinearColor(0.1f, 0.2f, 0.4f);
    NightSettings.FogDensity = 0.05f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AAtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    SetGoldenHourLighting();
}

void AAtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void AAtmosphericManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    UpdateLighting();
}

void AAtmosphericManager::SetGoldenHourLighting()
{
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    UpdateLighting();
}

void AAtmosphericManager::SetStormyWeather()
{
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
        FogComponent->SetFogDensity(0.08f);
        FogComponent->SetFogInscatteringColor(FLinearColor(0.3f, 0.3f, 0.4f));
    }

    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        LightComponent->SetLightColor(FLinearColor(0.6f, 0.6f, 0.7f));
        LightComponent->SetIntensity(1.5f);
    }

    WindStrength = 3.0f;
}

void AAtmosphericManager::SetClearWeather()
{
    if (LightingPresets.Contains(CurrentTimeOfDay))
    {
        const FEnvArt_LightingSettings& Settings = LightingPresets[CurrentTimeOfDay];
        
        if (AtmosphericFog && AtmosphericFog->GetComponent())
        {
            UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
            FogComponent->SetFogDensity(Settings.FogDensity);
            FogComponent->SetFogInscatteringColor(Settings.FogColor);
        }
    }

    WindStrength = 1.0f;
}

void AAtmosphericManager::UpdateLighting()
{
    if (!LightingPresets.Contains(CurrentTimeOfDay))
    {
        return;
    }

    const FEnvArt_LightingSettings& Settings = LightingPresets[CurrentTimeOfDay];

    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        LightComponent->SetLightColor(Settings.SunColor);
        LightComponent->SetIntensity(Settings.SunIntensity);
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    // Update atmospheric fog
    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
        FogComponent->SetFogDensity(Settings.FogDensity);
        FogComponent->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComponent->SetFogInscatteringColor(Settings.FogColor);
    }
}

void AAtmosphericManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }

    // Find exponential height fog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundFogs);
    if (FoundFogs.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FoundFogs[0]);
    }
}

void AAtmosphericManager::UpdateDayNightCycle(float DeltaTime)
{
    CurrentGameTime += DeltaTime * TimeMultiplier;
    
    if (CurrentGameTime >= 24.0f)
    {
        CurrentGameTime -= 24.0f;
    }

    EEnvArt_TimeOfDay NewTimeOfDay = GetTimeOfDayFromGameTime(CurrentGameTime);
    
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        CurrentTimeOfDay = NewTimeOfDay;
        UpdateLighting();
    }
}

void AAtmosphericManager::InterpolateLighting(const FEnvArt_LightingSettings& From, const FEnvArt_LightingSettings& To, float Alpha)
{
    FEnvArt_LightingSettings InterpolatedSettings;
    
    InterpolatedSettings.SunColor = FLinearColor::LerpUsingHSV(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.FogColor = FLinearColor::LerpUsingHSV(From.FogColor, To.FogColor, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    
    // Apply interpolated settings
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComponent = SunLight->GetLightComponent();
        LightComponent->SetLightColor(InterpolatedSettings.SunColor);
        LightComponent->SetIntensity(InterpolatedSettings.SunIntensity);
    }

    if (AtmosphericFog && AtmosphericFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComponent = AtmosphericFog->GetComponent();
        FogComponent->SetFogDensity(InterpolatedSettings.FogDensity);
        FogComponent->SetFogInscatteringColor(InterpolatedSettings.FogColor);
    }
}

EEnvArt_TimeOfDay AAtmosphericManager::GetTimeOfDayFromGameTime(float GameTime)
{
    if (GameTime >= 5.0f && GameTime < 7.0f)
    {
        return EEnvArt_TimeOfDay::Dawn;
    }
    else if (GameTime >= 7.0f && GameTime < 11.0f)
    {
        return EEnvArt_TimeOfDay::Morning;
    }
    else if (GameTime >= 11.0f && GameTime < 15.0f)
    {
        return EEnvArt_TimeOfDay::Noon;
    }
    else if (GameTime >= 15.0f && GameTime < 18.0f)
    {
        return EEnvArt_TimeOfDay::Afternoon;
    }
    else if (GameTime >= 18.0f && GameTime < 21.0f)
    {
        return EEnvArt_TimeOfDay::Dusk;
    }
    else
    {
        return EEnvArt_TimeOfDay::Night;
    }
}