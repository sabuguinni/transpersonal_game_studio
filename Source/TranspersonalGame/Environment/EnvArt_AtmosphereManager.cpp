#include "EnvArt_AtmosphereManager.h"
#include "Components/SceneComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/DirectionalLight.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    InitializeTimeOfDaySettings();
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindSceneLightingActors();
    ConfigureGoldenHourLighting();
}

void AEnvArt_AtmosphereManager::InitializeTimeOfDaySettings()
{
    // Dawn settings
    FEnvArt_LightingSettings DawnSettings;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f);
    DawnSettings.SunIntensity = 2.0f;
    DawnSettings.SunTemperature = 2800.0f;
    DawnSettings.SunRotation = FRotator(-20.0f, 80.0f, 0.0f);
    DawnSettings.FogDensity = 0.04f;
    DawnSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.6f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);

    // Morning settings
    FEnvArt_LightingSettings MorningSettings;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    MorningSettings.SunIntensity = 4.0f;
    MorningSettings.SunTemperature = 3500.0f;
    MorningSettings.SunRotation = FRotator(-10.0f, 60.0f, 0.0f);
    MorningSettings.FogDensity = 0.025f;
    MorningSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);

    // Noon settings
    FEnvArt_LightingSettings NoonSettings;
    NoonSettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f);
    NoonSettings.SunIntensity = 6.0f;
    NoonSettings.SunTemperature = 5500.0f;
    NoonSettings.SunRotation = FRotator(80.0f, 0.0f, 0.0f);
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.6f, 0.7f, 0.8f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Noon, NoonSettings);

    // Afternoon (Golden Hour) settings
    FEnvArt_LightingSettings AfternoonSettings;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f);
    AfternoonSettings.SunIntensity = 3.5f;
    AfternoonSettings.SunTemperature = 3200.0f;
    AfternoonSettings.SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
    AfternoonSettings.FogDensity = 0.02f;
    AfternoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk settings
    FEnvArt_LightingSettings DuskSettings;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f);
    DuskSettings.SunIntensity = 1.5f;
    DuskSettings.SunTemperature = 2500.0f;
    DuskSettings.SunRotation = FRotator(-25.0f, 30.0f, 0.0f);
    DuskSettings.FogDensity = 0.035f;
    DuskSettings.FogColor = FLinearColor(0.8f, 0.6f, 0.5f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);

    // Night settings
    FEnvArt_LightingSettings NightSettings;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f);
    NightSettings.SunIntensity = 0.5f;
    NightSettings.SunTemperature = 4000.0f;
    NightSettings.SunRotation = FRotator(-80.0f, 0.0f, 0.0f);
    NightSettings.FogDensity = 0.05f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void AEnvArt_AtmosphereManager::FindSceneLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        SunLight = *ActorItr;
        break; // Use first found
    }

    // Find fog actor
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        FogActor = *ActorItr;
        break; // Use first found
    }

    // Find sky atmosphere
    for (TActorIterator<ASkyAtmosphere> ActorItr(World); ActorItr; ++ActorItr)
    {
        SkyAtmosphereActor = *ActorItr;
        break; // Use first found
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (TimeOfDaySettings.Contains(NewTimeOfDay))
    {
        ApplyLightingSettings(TimeOfDaySettings[NewTimeOfDay]);
    }
}

void AEnvArt_AtmosphereManager::ApplyLightingSettings(const FEnvArt_LightingSettings& Settings)
{
    // Configure sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetLightColor(Settings.SunColor);
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetTemperature(Settings.SunTemperature);
        SunLight->SetActorRotation(Settings.SunRotation);
    }

    // Configure atmospheric fog
    ConfigureAtmosphericFog(Settings.FogDensity, Settings.FogHeightFalloff, Settings.FogColor);
}

void AEnvArt_AtmosphereManager::ConfigureGoldenHourLighting()
{
    SetTimeOfDay(EEnvArt_TimeOfDay::Afternoon);
}

void AEnvArt_AtmosphereManager::ConfigureAtmosphericFog(float Density, float HeightFalloff, FLinearColor Color)
{
    if (FogActor && FogActor->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
        FogComp->SetFogDensity(Density);
        FogComp->SetFogHeightFalloff(HeightFalloff);
        FogComp->SetFogInscatteringColor(Color);
    }
}

void AEnvArt_AtmosphereManager::RefreshAtmosphereSettings()
{
    FindSceneLightingActors();
    ApplyLightingSettings(TimeOfDaySettings[CurrentTimeOfDay]);
}