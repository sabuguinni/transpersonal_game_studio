#include "EnvArt_AtmosphereController.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"

AEnvArt_AtmosphereController::AEnvArt_AtmosphereController()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;
    TimeOfDayProgress = 0.0f;
    bEnableVolumetricFog = true;
    bEnableGodRays = true;
    ParticleIntensity = 0.5f;

    InitializeLightingPresets();
}

void AEnvArt_AtmosphereController::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    SetGoldenHourLighting();
}

void AEnvArt_AtmosphereController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-update lighting if needed
    if (TimeOfDayProgress > 0.0f)
    {
        UpdateLighting();
    }
}

void AEnvArt_AtmosphereController::InitializeLightingPresets()
{
    LightingPresets.Empty();

    // Dawn
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunRotation = FRotator(-10.0f, 80.0f, 0.0f);
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogInscatteringColor = FLinearColor(0.9f, 0.7f, 0.6f, 1.0f);
    DawnSettings.AtmosphereHaziness = 0.6f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);

    // Morning
    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunRotation = FRotator(20.0f, 120.0f, 0.0f);
    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.FogDensity = 0.03f;
    MorningSettings.FogInscatteringColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    MorningSettings.AtmosphereHaziness = 0.3f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);

    // Noon
    FEnvArt_LightingSettings NoonSettings;
    NoonSettings.SunRotation = FRotator(80.0f, 180.0f, 0.0f);
    NoonSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
    NoonSettings.SunIntensity = 6.0f;
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogInscatteringColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
    NoonSettings.AtmosphereHaziness = 0.2f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Noon, NoonSettings);

    // Afternoon (Golden Hour)
    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunRotation = FRotator(15.0f, 240.0f, 0.0f);
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonSettings.SunIntensity = 3.5f;
    AfternoonSettings.FogDensity = 0.025f;
    AfternoonSettings.FogInscatteringColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    AfternoonSettings.AtmosphereHaziness = 0.5f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk
    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunRotation = FRotator(-5.0f, 280.0f, 0.0f);
    DuskSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogInscatteringColor = FLinearColor(0.8f, 0.6f, 0.7f, 1.0f);
    DuskSettings.AtmosphereHaziness = 0.7f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);

    // Night
    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunRotation = FRotator(-45.0f, 0.0f, 0.0f);
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.1f;
    NightSettings.FogDensity = 0.02f;
    NightSettings.FogInscatteringColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    NightSettings.AtmosphereHaziness = 0.8f;
    LightingPresets.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AEnvArt_AtmosphereController::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> DirectionalLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirectionalLights);
    if (DirectionalLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(DirectionalLights[0]);
    }

    // Find fog actor
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() > 0)
    {
        AtmosphericFog = Cast<AExponentialHeightFog>(FogActors[0]);
    }
}

void AEnvArt_AtmosphereController::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    UpdateLighting();
}

void AEnvArt_AtmosphereController::SetGoldenHourLighting()
{
    SetTimeOfDay(EEnvArt_TimeOfDay::Afternoon);
}

void AEnvArt_AtmosphereController::UpdateLighting()
{
    if (!LightingPresets.Contains(CurrentTimeOfDay)) return;

    FEnvArt_LightingSettings Settings = LightingPresets[CurrentTimeOfDay];
    ApplyLightingSettings(Settings);
}

FEnvArt_LightingSettings AEnvArt_AtmosphereController::GetCurrentLightingSettings() const
{
    if (LightingPresets.Contains(CurrentTimeOfDay))
    {
        return LightingPresets[CurrentTimeOfDay];
    }
    return FEnvArt_LightingSettings();
}

void AEnvArt_AtmosphereController::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    // Apply sun lighting
    if (SunLight.IsValid())
    {
        ADirectionalLight* Sun = SunLight.Get();
        Sun->SetActorRotation(Settings.SunRotation);
        
        UDirectionalLightComponent* LightComp = Sun->GetLightComponent();
        if (LightComp)
        {
            LightComp->SetLightColor(Settings.SunColor);
            LightComp->SetIntensity(Settings.SunIntensity);
        }
    }

    // Apply fog settings
    if (AtmosphericFog.IsValid())
    {
        AExponentialHeightFog* Fog = AtmosphericFog.Get();
        UExponentialHeightFogComponent* FogComp = Fog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Settings.FogDensity);
            FogComp->SetFogInscatteringColor(Settings.FogInscatteringColor);
            FogComp->SetVolumetricFog(bEnableVolumetricFog);
        }
    }
}

void AEnvArt_AtmosphereController::InterpolateLighting(const FEnvArt_LightingSettings& From, const FEnvArt_LightingSettings& To, float Alpha)
{
    FEnvArt_LightingSettings InterpolatedSettings;
    
    // Interpolate rotation
    InterpolatedSettings.SunRotation = FMath::Lerp(From.SunRotation, To.SunRotation, Alpha);
    
    // Interpolate colors
    InterpolatedSettings.SunColor = FMath::Lerp(From.SunColor, To.SunColor, Alpha);
    InterpolatedSettings.FogInscatteringColor = FMath::Lerp(From.FogInscatteringColor, To.FogInscatteringColor, Alpha);
    
    // Interpolate scalars
    InterpolatedSettings.SunIntensity = FMath::Lerp(From.SunIntensity, To.SunIntensity, Alpha);
    InterpolatedSettings.FogDensity = FMath::Lerp(From.FogDensity, To.FogDensity, Alpha);
    InterpolatedSettings.AtmosphereHaziness = FMath::Lerp(From.AtmosphereHaziness, To.AtmosphereHaziness, Alpha);
    
    ApplyLightingSettings(InterpolatedSettings);
}