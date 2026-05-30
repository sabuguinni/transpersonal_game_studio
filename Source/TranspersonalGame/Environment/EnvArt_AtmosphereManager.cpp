#include "EnvArt_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

UEnvArt_AtmosphereManager::UEnvArt_AtmosphereManager()
{
    CurrentTimeOfDay = EEnvArt_TimeOfDay::Morning;
    SunLight = nullptr;
    SkyLight = nullptr;
    GlobalFog = nullptr;
}

void UEnvArt_AtmosphereManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    InitializeAtmosphereActors();
    ConfigureDefaultTimeOfDaySettings();
    
    // Set initial golden hour lighting for prehistoric atmosphere
    SetGoldenHourLighting();
    EnableVolumetricFog(true);
}

void UEnvArt_AtmosphereManager::Deinitialize()
{
    SunLight = nullptr;
    SkyLight = nullptr;
    GlobalFog = nullptr;
    
    Super::Deinitialize();
}

void UEnvArt_AtmosphereManager::InitializeAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find existing atmosphere actors
    for (TActorIterator<ADirectionalLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        SunLight = *ActorIterator;
        break;
    }

    for (TActorIterator<ASkyLight> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        SkyLight = *ActorIterator;
        break;
    }

    for (TActorIterator<AExponentialHeightFog> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        GlobalFog = *ActorIterator;
        break;
    }

    // Create missing atmosphere actors if needed
    if (!SunLight)
    {
        SunLight = World->SpawnActor<ADirectionalLight>();
        if (SunLight)
        {
            SunLight->SetActorLabel(TEXT("Sun_Prehistoric"));
        }
    }

    if (!SkyLight)
    {
        SkyLight = World->SpawnActor<ASkyLight>();
        if (SkyLight)
        {
            SkyLight->SetActorLabel(TEXT("Sky_Prehistoric"));
        }
    }

    if (!GlobalFog)
    {
        GlobalFog = World->SpawnActor<AExponentialHeightFog>();
        if (GlobalFog)
        {
            GlobalFog->SetActorLabel(TEXT("AtmosphericFog_Global"));
        }
    }
}

void UEnvArt_AtmosphereManager::ConfigureDefaultTimeOfDaySettings()
{
    // Dawn settings
    FEnvArt_AtmosphereSettings DawnSettings;
    DawnSettings.SunIntensity = 1.5f;
    DawnSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
    DawnSettings.SunAngle = 15.0f;
    DawnSettings.FogDensity = 0.05f;
    DawnSettings.FogColor = FLinearColor(0.9f, 0.8f, 0.7f, 1.0f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dawn, DawnSettings);

    // Morning settings (golden hour)
    FEnvArt_AtmosphereSettings MorningSettings;
    MorningSettings.SunIntensity = 2.5f;
    MorningSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    MorningSettings.SunAngle = 30.0f;
    MorningSettings.FogDensity = 0.02f;
    MorningSettings.FogColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Morning, MorningSettings);

    // Noon settings
    FEnvArt_AtmosphereSettings NoonSettings;
    NoonSettings.SunIntensity = 4.0f;
    NoonSettings.SunColor = FLinearColor::White;
    NoonSettings.SunAngle = 80.0f;
    NoonSettings.FogDensity = 0.01f;
    NoonSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Noon, NoonSettings);

    // Afternoon settings
    FEnvArt_AtmosphereSettings AfternoonSettings;
    AfternoonSettings.SunIntensity = 3.0f;
    AfternoonSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
    AfternoonSettings.SunAngle = 45.0f;
    AfternoonSettings.FogDensity = 0.015f;
    AfternoonSettings.FogColor = FLinearColor(0.8f, 0.75f, 0.6f, 1.0f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Afternoon, AfternoonSettings);

    // Dusk settings
    FEnvArt_AtmosphereSettings DuskSettings;
    DuskSettings.SunIntensity = 1.8f;
    DuskSettings.SunColor = FLinearColor(1.0f, 0.6f, 0.4f, 1.0f);
    DuskSettings.SunAngle = 10.0f;
    DuskSettings.FogDensity = 0.04f;
    DuskSettings.FogColor = FLinearColor(0.9f, 0.6f, 0.4f, 1.0f);
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Dusk, DuskSettings);

    // Night settings
    FEnvArt_AtmosphereSettings NightSettings;
    NightSettings.SunIntensity = 0.1f;
    NightSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f);
    NightSettings.SunAngle = -20.0f;
    NightSettings.FogDensity = 0.03f;
    NightSettings.FogColor = FLinearColor(0.2f, 0.3f, 0.5f, 1.0f);
    NightSettings.SkyLightIntensity = 0.3f;
    TimeOfDaySettings.Add(EEnvArt_TimeOfDay::Night, NightSettings);
}

void UEnvArt_AtmosphereManager::SetTimeOfDay(EEnvArt_TimeOfDay NewTimeOfDay)
{
    CurrentTimeOfDay = NewTimeOfDay;
    
    if (TimeOfDaySettings.Contains(NewTimeOfDay))
    {
        UpdateAtmosphere(TimeOfDaySettings[NewTimeOfDay]);
    }
}

void UEnvArt_AtmosphereManager::UpdateAtmosphere(const FEnvArt_AtmosphereSettings& Settings)
{
    // Update sun light
    if (SunLight && SunLight->GetLightComponent())
    {
        UDirectionalLightComponent* LightComp = SunLight->GetLightComponent();
        LightComp->SetIntensity(Settings.SunIntensity);
        LightComp->SetLightColor(Settings.SunColor);
        
        // Set sun rotation based on angle
        FRotator SunRotation(Settings.SunAngle, 0.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
    }

    // Update sky light
    if (SkyLight && SkyLight->GetLightComponent())
    {
        USkyLightComponent* SkyComp = SkyLight->GetLightComponent();
        SkyComp->SetIntensity(Settings.SkyLightIntensity);
        SkyComp->SetLightColor(Settings.SkyColor);
    }

    // Update global fog
    if (GlobalFog && GlobalFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = GlobalFog->GetComponent();
        FogComp->SetFogDensity(Settings.FogDensity);
        FogComp->SetFogHeightFalloff(Settings.FogHeightFalloff);
        FogComp->SetFogInscatteringColor(Settings.FogColor);
    }
}

void UEnvArt_AtmosphereManager::CreateBiomeSpecificFog(const FVector& BiomeCenter, float Radius, const FLinearColor& FogColor)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create localized fog for specific biome
    AExponentialHeightFog* BiomeFog = World->SpawnActor<AExponentialHeightFog>();
    if (BiomeFog)
    {
        BiomeFog->SetActorLocation(BiomeCenter);
        BiomeFog->SetActorLabel(TEXT("BiomeFog_Local"));
        
        if (UExponentialHeightFogComponent* FogComp = BiomeFog->GetComponent())
        {
            FogComp->SetFogDensity(0.03f);
            FogComp->SetFogHeightFalloff(0.15f);
            FogComp->SetFogInscatteringColor(FogColor);
            FogComp->SetFogMaxOpacity(0.8f);
        }
    }
}

void UEnvArt_AtmosphereManager::SetGoldenHourLighting()
{
    FEnvArt_AtmosphereSettings GoldenHourSettings;
    GoldenHourSettings.SunIntensity = 2.8f;
    GoldenHourSettings.SunColor = FLinearColor(1.0f, 0.85f, 0.6f, 1.0f);
    GoldenHourSettings.SunAngle = 25.0f;
    GoldenHourSettings.FogDensity = 0.025f;
    GoldenHourSettings.FogColor = FLinearColor(0.9f, 0.75f, 0.5f, 1.0f);
    GoldenHourSettings.SkyLightIntensity = 1.2f;
    GoldenHourSettings.SkyColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    
    UpdateAtmosphere(GoldenHourSettings);
}

void UEnvArt_AtmosphereManager::EnableVolumetricFog(bool bEnable)
{
    if (GlobalFog && GlobalFog->GetComponent())
    {
        UExponentialHeightFogComponent* FogComp = GlobalFog->GetComponent();
        FogComp->SetVolumetricFog(bEnable);
        
        if (bEnable)
        {
            FogComp->SetVolumetricFogScatteringDistribution(0.2f);
            FogComp->SetVolumetricFogAlbedo(FLinearColor(0.9f, 0.9f, 0.8f, 1.0f));
            FogComp->SetVolumetricFogEmissive(FLinearColor(0.1f, 0.08f, 0.05f, 1.0f));
        }
    }
}

FEnvArt_AtmosphereSettings UEnvArt_AtmosphereManager::GetSettingsForTimeOfDay(EEnvArt_TimeOfDay TimeOfDay) const
{
    if (TimeOfDaySettings.Contains(TimeOfDay))
    {
        return TimeOfDaySettings[TimeOfDay];
    }
    
    return FEnvArt_AtmosphereSettings();
}