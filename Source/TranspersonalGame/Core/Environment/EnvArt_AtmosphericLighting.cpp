#include "EnvArt_AtmosphericLighting.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UEnvArt_AtmosphericLighting::UEnvArt_AtmosphericLighting()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;

    // Initialize lighting presets
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunRotation = FRotator(-10.0f, 45.0f, 0.0f);
    DawnSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.9f, 1.0f);
    DawnSettings.FogDensity = 0.05f;

    MorningSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    MorningSettings.SunIntensity = 3.0f;
    MorningSettings.SunRotation = FRotator(-30.0f, 60.0f, 0.0f);
    MorningSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);
    MorningSettings.FogDensity = 0.02f;

    NoonSettings.SunColor = FLinearColor::White;
    NoonSettings.SunIntensity = 5.0f;
    NoonSettings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    NoonSettings.FogColor = FLinearColor(0.5f, 0.6f, 0.8f, 1.0f);
    NoonSettings.FogDensity = 0.01f;

    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AfternoonSettings.SunIntensity = 4.0f;
    AfternoonSettings.SunRotation = FRotator(-45.0f, -45.0f, 0.0f);
    AfternoonSettings.FogColor = FLinearColor(0.7f, 0.6f, 0.5f, 1.0f);
    AfternoonSettings.FogDensity = 0.015f;

    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
    DuskSettings.SunIntensity = 2.0f;
    DuskSettings.SunRotation = FRotator(-5.0f, -60.0f, 0.0f);
    DuskSettings.FogColor = FLinearColor(0.9f, 0.5f, 0.4f, 1.0f);
    DuskSettings.FogDensity = 0.03f;

    NightSettings.SunColor = FLinearColor(0.2f, 0.3f, 0.6f, 1.0f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunRotation = FRotator(10.0f, 0.0f, 0.0f);
    NightSettings.FogColor = FLinearColor(0.1f, 0.2f, 0.4f, 1.0f);
    NightSettings.FogDensity = 0.08f;
}

void UEnvArt_AtmosphericLighting::BeginPlay()
{
    Super::BeginPlay();
    
    FindAtmosphericComponents();
    InitializeAtmosphericComponents();
    ApplyLightingSettings(GetCurrentLightingSettings());
}

void UEnvArt_AtmosphericLighting::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsTransitioning)
    {
        UpdateTransition(DeltaTime);
    }

    if (bAutoAdvanceTime)
    {
        // Auto-advance time of day (placeholder for future implementation)
        // This would integrate with game time system
    }
}

void UEnvArt_AtmosphericLighting::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    if (NewTimeOfDay == CurrentTimeOfDay)
    {
        return;
    }

    CurrentTimeOfDay = NewTimeOfDay;
    
    if (bSmoothTransitions)
    {
        StartSettings = GetCurrentLightingSettings();
        TargetSettings = GetCurrentLightingSettings();
        CurrentTransitionTime = 0.0f;
        bIsTransitioning = true;
    }
    else
    {
        ApplyLightingSettings(GetCurrentLightingSettings());
    }
}

void UEnvArt_AtmosphericLighting::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetIntensity(Settings.SunIntensity);
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    if (HeightFog)
    {
        HeightFog->SetFogInscatteringColor(Settings.FogColor);
        HeightFog->SetFogDensity(Settings.FogDensity);
        HeightFog->SetFogHeightFalloff(Settings.FogHeightFalloff);
    }
}

FEnvArt_LightingSettings UEnvArt_AtmosphericLighting::GetCurrentLightingSettings() const
{
    switch (CurrentTimeOfDay)
    {
        case EEnvArt_TimeOfDay::Dawn:
            return DawnSettings;
        case EEnvArt_TimeOfDay::Morning:
            return MorningSettings;
        case EEnvArt_TimeOfDay::Noon:
            return NoonSettings;
        case EEnvArt_TimeOfDay::Afternoon:
            return AfternoonSettings;
        case EEnvArt_TimeOfDay::Dusk:
            return DuskSettings;
        case EEnvArt_TimeOfDay::Night:
            return NightSettings;
        default:
            return MorningSettings;
    }
}

void UEnvArt_AtmosphericLighting::SetGoldenHourLighting()
{
    FEnvArt_LightingSettings GoldenHour;
    GoldenHour.SunColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f);
    GoldenHour.SunIntensity = 3.5f;
    GoldenHour.SunRotation = FRotator(-20.0f, 45.0f, 0.0f);
    GoldenHour.FogColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
    GoldenHour.FogDensity = 0.025f;
    GoldenHour.FogHeightFalloff = 0.15f;

    ApplyLightingSettings(GoldenHour);
}

void UEnvArt_AtmosphericLighting::SetVolumetricFogIntensity(float Intensity)
{
    if (HeightFog)
    {
        HeightFog->SetVolumetricFogScatteringDistribution(FMath::Clamp(Intensity, 0.0f, 1.0f));
        HeightFog->SetVolumetricFogExtinctionScale(Intensity * 2.0f);
    }
}

void UEnvArt_AtmosphericLighting::InitializeAtmosphericComponents()
{
    if (!SunLight || !HeightFog)
    {
        FindAtmosphericComponents();
    }

    // Initialize sky atmosphere if available
    if (SkyAtmosphere)
    {
        // Configure sky atmosphere for realistic lighting
        SkyAtmosphere->SetAtmosphereHeight(60.0f);
        SkyAtmosphere->SetMultiScatteringFactor(1.0f);
    }
}

void UEnvArt_AtmosphericLighting::UpdateTransition(float DeltaTime)
{
    CurrentTransitionTime += DeltaTime;
    float Alpha = FMath::Clamp(CurrentTransitionTime / TransitionDuration, 0.0f, 1.0f);

    FEnvArt_LightingSettings CurrentSettings = LerpLightingSettings(StartSettings, TargetSettings, Alpha);
    ApplyLightingSettings(CurrentSettings);

    if (Alpha >= 1.0f)
    {
        bIsTransitioning = false;
        CurrentTransitionTime = 0.0f;
    }
}

FEnvArt_LightingSettings UEnvArt_AtmosphericLighting::LerpLightingSettings(const FEnvArt_LightingSettings& A, const FEnvArt_LightingSettings& B, float Alpha) const
{
    FEnvArt_LightingSettings Result;
    Result.SunColor = FLinearColor::LerpUsingHSV(A.SunColor, B.SunColor, Alpha);
    Result.SunIntensity = FMath::Lerp(A.SunIntensity, B.SunIntensity, Alpha);
    Result.SunRotation = FMath::Lerp(A.SunRotation, B.SunRotation, Alpha);
    Result.FogColor = FLinearColor::LerpUsingHSV(A.FogColor, B.FogColor, Alpha);
    Result.FogDensity = FMath::Lerp(A.FogDensity, B.FogDensity, Alpha);
    Result.FogHeightFalloff = FMath::Lerp(A.FogHeightFalloff, B.FogHeightFalloff, Alpha);
    return Result;
}

void UEnvArt_AtmosphericLighting::FindAtmosphericComponents()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find directional light (sun)
    if (!SunLight)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
        if (FoundActors.Num() > 0)
        {
            SunLight = Cast<ADirectionalLight>(FoundActors[0]);
        }
    }

    // Find height fog component
    if (!HeightFog)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                UExponentialHeightFogComponent* FogComp = Actor->FindComponentByClass<UExponentialHeightFogComponent>();
                if (FogComp)
                {
                    HeightFog = FogComp;
                    break;
                }
            }
        }
    }

    // Find sky atmosphere component
    if (!SkyAtmosphere)
    {
        for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AActor* Actor = *ActorIterator;
            if (Actor)
            {
                USkyAtmosphereComponent* SkyComp = Actor->FindComponentByClass<USkyAtmosphereComponent>();
                if (SkyComp)
                {
                    SkyAtmosphere = SkyComp;
                    break;
                }
            }
        }
    }
}