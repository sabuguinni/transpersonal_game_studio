#include "Light_AtmosphericSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

ALight_AtmosphericSystem::ALight_AtmosphericSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default Cretaceous settings
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    AtmosphericSettings.SkyLightIntensity = 1.5f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogHeightFalloff = 0.2f;
    AtmosphericSettings.FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AtmosphericSettings.bVolumetricFog = true;
    AtmosphericSettings.VolumetricScatteringDistribution = 0.2f;
}

void ALight_AtmosphericSystem::BeginPlay()
{
    Super::BeginPlay();
    
    FindAndConfigureLightingActors();
    SetCretaceousLighting();
}

void ALight_AtmosphericSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableDayNightCycle)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_AtmosphericSystem::SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;

    switch (CurrentTimeOfDay)
    {
        case ELight_TimeOfDay::Dawn:
            AtmosphericSettings.SunIntensity = 2.0f;
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
            break;
        case ELight_TimeOfDay::Morning:
            AtmosphericSettings.SunIntensity = 4.0f;
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
            break;
        case ELight_TimeOfDay::Noon:
            AtmosphericSettings.SunIntensity = 6.0f;
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.98f, 0.9f, 1.0f);
            break;
        case ELight_TimeOfDay::Afternoon:
            AtmosphericSettings.SunIntensity = 5.0f;
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
            break;
        case ELight_TimeOfDay::Dusk:
            AtmosphericSettings.SunIntensity = 1.5f;
            AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
            break;
        case ELight_TimeOfDay::Night:
            AtmosphericSettings.SunIntensity = 0.1f;
            AtmosphericSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
            break;
    }

    ApplyAtmosphericSettings(AtmosphericSettings);
}

void ALight_AtmosphericSystem::ApplyAtmosphericSettings(const FLight_AtmosphericSettings& Settings)
{
    AtmosphericSettings = Settings;
    
    ConfigureDirectionalLight();
    ConfigureSkyLight();
    ConfigureHeightFog();
}

void ALight_AtmosphericSystem::SetCretaceousLighting()
{
    // Set optimal Cretaceous period lighting
    AtmosphericSettings.SunIntensity = 5.0f;
    AtmosphericSettings.SunColor = FLinearColor(1.0f, 0.94f, 0.78f, 1.0f);
    AtmosphericSettings.SkyLightIntensity = 1.5f;
    AtmosphericSettings.FogDensity = 0.02f;
    AtmosphericSettings.FogInscatteringColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    
    ApplyAtmosphericSettings(AtmosphericSettings);
}

void ALight_AtmosphericSystem::UpdateVolumetricFog(bool bEnable, float Density)
{
    AtmosphericSettings.bVolumetricFog = bEnable;
    AtmosphericSettings.FogDensity = Density;
    
    ConfigureHeightFog();
}

void ALight_AtmosphericSystem::FindAndConfigureLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find DirectionalLight
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break;
    }

    // Find SkyLight
    for (TActorIterator<ASkyLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyLightActor = *ActorItr;
        break;
    }

    // Find ExponentialHeightFog
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActor = *ActorItr;
        break;
    }
}

void ALight_AtmosphericSystem::UpdateDayNightCycle(float DeltaTime)
{
    CurrentCycleTime += DeltaTime;
    float CycleDuration = DayDurationMinutes * 60.0f; // Convert to seconds
    
    if (CurrentCycleTime >= CycleDuration)
    {
        CurrentCycleTime = 0.0f;
    }

    float CycleProgress = CurrentCycleTime / CycleDuration;
    
    // Map cycle progress to time of day
    if (CycleProgress < 0.1f) // Dawn
    {
        SetTimeOfDay(ELight_TimeOfDay::Dawn);
    }
    else if (CycleProgress < 0.3f) // Morning
    {
        SetTimeOfDay(ELight_TimeOfDay::Morning);
    }
    else if (CycleProgress < 0.5f) // Noon
    {
        SetTimeOfDay(ELight_TimeOfDay::Noon);
    }
    else if (CycleProgress < 0.7f) // Afternoon
    {
        SetTimeOfDay(ELight_TimeOfDay::Afternoon);
    }
    else if (CycleProgress < 0.85f) // Dusk
    {
        SetTimeOfDay(ELight_TimeOfDay::Dusk);
    }
    else // Night
    {
        SetTimeOfDay(ELight_TimeOfDay::Night);
    }
}

void ALight_AtmosphericSystem::ConfigureDirectionalLight()
{
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(AtmosphericSettings.SunIntensity);
        LightComp->SetLightColor(AtmosphericSettings.SunColor);
        LightComp->SetCastVolumetricShadow(true);
        LightComp->SetVolumetricScatteringIntensity(1.0f);
    }
}

void ALight_AtmosphericSystem::ConfigureSkyLight()
{
    if (SkyLightActor && SkyLightActor->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLightActor->GetLightComponent();
        SkyComp->SetIntensity(AtmosphericSettings.SkyLightIntensity);
        SkyComp->SetLightColor(FLinearColor(0.78f, 0.86f, 1.0f, 1.0f));
    }
}

void ALight_AtmosphericSystem::ConfigureHeightFog()
{
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(AtmosphericSettings.FogDensity);
        FogComp->SetFogHeightFalloff(AtmosphericSettings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(AtmosphericSettings.FogInscatteringColor);
        FogComp->SetVolumetricFog(AtmosphericSettings.bVolumetricFog);
        FogComp->SetVolumetricFogScatteringDistribution(AtmosphericSettings.VolumetricScatteringDistribution);
    }
}