#include "EnvArt_AtmosphereController.h"
#include "Engine/Engine.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create directional light component (sun)
    SunLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("SunLight"));
    SunLight->SetupAttachment(RootComponent);
    SunLight->SetIntensity(3.0f);
    SunLight->SetLightColor(FLinearColor(1.0f, 0.9f, 0.7f, 1.0f));
    SunLight->SetRelativeRotation(FRotator(-30.0f, 45.0f, 0.0f));
    SunLight->SetCastShadows(true);
    SunLight->SetCastVolumetricShadow(true);

    // Create sky light component
    SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
    SkyLight->SetupAttachment(RootComponent);
    SkyLight->SetIntensity(1.0f);
    SkyLight->SetLightColor(FLinearColor(0.3f, 0.6f, 1.0f, 1.0f));
    SkyLight->SetSourceType(ESkyLightSourceType::SLS_CapturedScene);

    // Create exponential height fog component
    HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
    HeightFog->SetupAttachment(RootComponent);
    HeightFog->SetFogDensity(0.02f);
    HeightFog->SetFogHeightFalloff(0.2f);
    HeightFog->SetFogInscatteringColor(FLinearColor(0.8f, 0.9f, 1.0f, 1.0f));
    HeightFog->SetVolumetricFog(true);
    HeightFog->SetVolumetricFogScatteringDistribution(0.2f);
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAtmospherePresets();
    SetTimeOfDay(CurrentTimeOfDay);
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDynamicTimeOfDay)
    {
        UpdateDynamicTimeOfDay(DeltaTime);
    }
}

void AEnvArt_AtmosphereController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (AtmospherePresets.Contains(NewTimeOfDay))
    {
        ApplyAtmosphereSettings(AtmospherePresets[NewTimeOfDay]);
    }
}

void AEnvArt_AtmosphereController::ApplyAtmosphereSettings(const FEnvArt_AtmosphereSettings& Settings)
{
    if (SunLight)
    {
        SunLight->SetLightColor(Settings.SunColor);
        SunLight->SetIntensity(Settings.SunIntensity);
        SunLight->SetRelativeRotation(Settings.SunRotation);
    }

    if (SkyLight)
    {
        SkyLight->SetLightColor(Settings.SkyColor);
        SkyLight->SetIntensity(Settings.SkyIntensity);
        SkyLight->RecaptureSky();
    }

    if (HeightFog)
    {
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }
}

void AEnvArt_AtmosphereController::CreateGoldenHourAtmosphere()
{
    FEnvArt_AtmosphereSettings GoldenHour;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);
    GoldenHour.SunIntensity = 2.5f;
    GoldenHour.SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
    GoldenHour.FogColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    GoldenHour.FogDensity = 0.015f;
    GoldenHour.SkyColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f);
    GoldenHour.SkyIntensity = 0.8f;
    
    ApplyAtmosphereSettings(GoldenHour);
}

void AEnvArt_AtmosphereController::CreateMorningMistAtmosphere()
{
    FEnvArt_AtmosphereSettings MorningMist;
    MorningMist.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningMist.SunIntensity = 2.0f;
    MorningMist.SunRotation = FRotator(-20.0f, 30.0f, 0.0f);
    MorningMist.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    MorningMist.FogDensity = 0.04f;
    MorningMist.FogHeightFalloff = 0.15f;
    MorningMist.SkyColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    MorningMist.SkyIntensity = 1.2f;
    
    ApplyAtmosphereSettings(MorningMist);
}

void AEnvArt_AtmosphereController::CreateDramaticStormAtmosphere()
{
    FEnvArt_AtmosphereSettings Storm;
    Storm.SunColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    Storm.SunIntensity = 1.0f;
    Storm.SunRotation = FRotator(-45.0f, 60.0f, 0.0f);
    Storm.FogColor = FLinearColor(0.4f, 0.5f, 0.6f, 1.0f);
    Storm.FogDensity = 0.08f;
    Storm.FogHeightFalloff = 0.3f;
    Storm.SkyColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);
    Storm.SkyIntensity = 0.5f;
    
    ApplyAtmosphereSettings(Storm);
}

void AEnvArt_AtmosphereController::InitializeAtmospherePresets()
{
    // Dawn
    FEnvArt_AtmosphereSettings Dawn;
    Dawn.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    Dawn.SunIntensity = 1.5f;
    Dawn.SunRotation = FRotator(-10.0f, 15.0f, 0.0f);
    Dawn.FogColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    Dawn.FogDensity = 0.03f;
    Dawn.SkyColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    Dawn.SkyIntensity = 0.6f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Dawn, Dawn);

    // Morning
    FEnvArt_AtmosphereSettings Morning;
    Morning.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    Morning.SunIntensity = 2.5f;
    Morning.SunRotation = FRotator(-25.0f, 30.0f, 0.0f);
    Morning.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    Morning.FogDensity = 0.025f;
    Morning.SkyColor = FLinearColor(0.6f, 0.8f, 1.0f, 1.0f);
    Morning.SkyIntensity = 0.8f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Morning, Morning);

    // Noon
    FEnvArt_AtmosphereSettings Noon;
    Noon.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    Noon.SunIntensity = 4.0f;
    Noon.SunRotation = FRotator(-80.0f, 45.0f, 0.0f);
    Noon.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    Noon.FogDensity = 0.015f;
    Noon.SkyColor = FLinearColor(0.3f, 0.6f, 1.0f, 1.0f);
    Noon.SkyIntensity = 1.0f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Noon, Noon);

    // Afternoon
    FEnvArt_AtmosphereSettings Afternoon;
    Afternoon.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    Afternoon.SunIntensity = 3.0f;
    Afternoon.SunRotation = FRotator(-40.0f, 60.0f, 0.0f);
    Afternoon.FogColor = FLinearColor(0.9f, 0.9f, 0.8f, 1.0f);
    Afternoon.FogDensity = 0.02f;
    Afternoon.SkyColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
    Afternoon.SkyIntensity = 0.9f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Afternoon, Afternoon);

    // Dusk
    FEnvArt_AtmosphereSettings Dusk;
    Dusk.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    Dusk.SunIntensity = 2.0f;
    Dusk.SunRotation = FRotator(-5.0f, 75.0f, 0.0f);
    Dusk.FogColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    Dusk.FogDensity = 0.035f;
    Dusk.SkyColor = FLinearColor(1.0f, 0.5f, 0.3f, 1.0f);
    Dusk.SkyIntensity = 0.7f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Dusk, Dusk);

    // Night
    FEnvArt_AtmosphereSettings Night;
    Night.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    Night.SunIntensity = 0.1f;
    Night.SunRotation = FRotator(30.0f, 90.0f, 0.0f);
    Night.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    Night.FogDensity = 0.05f;
    Night.SkyColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    Night.SkyIntensity = 0.3f;
    AtmospherePresets.Add(EEnvArt_TimeOfDay::Night, Night);
}

void AEnvArt_AtmosphereController::UpdateDynamicTimeOfDay(float DeltaTime)
{
    CurrentTimeOfDayFloat += DeltaTime * TimeOfDaySpeed;
    
    if (CurrentTimeOfDayFloat >= 6.0f)
    {
        CurrentTimeOfDayFloat = 0.0f;
    }

    EEnvArt_TimeOfDay NewTimeOfDay = static_cast<EEnvArt_TimeOfDay>(FMath::FloorToInt(CurrentTimeOfDayFloat));
    
    if (NewTimeOfDay != CurrentTimeOfDay)
    {
        SetTimeOfDay(NewTimeOfDay);
    }
}