#include "BiomeLightingManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/VolumetricCloud.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Kismet/GameplayStatics.h"

ALight_BiomeLightingManager::ALight_BiomeLightingManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    SunLight = nullptr;
    SkyLight = nullptr;
    HeightFog = nullptr;
    SkyAtmosphere = nullptr;
    VolumetricClouds = nullptr;
    
    SetupBiomePresets();
}

void ALight_BiomeLightingManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeLightingActors();
    ApplyBiomeLighting();
}

void ALight_BiomeLightingManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bAutoAdvanceTime)
    {
        UpdateDayNightCycle(DeltaTime);
    }
}

void ALight_BiomeLightingManager::SetBiome(ELight_BiomeType NewBiome)
{
    CurrentBiome = NewBiome;
    ApplyBiomeLighting();
}

void ALight_BiomeLightingManager::SetTimeOfDay(float Hours)
{
    TimeOfDay = FMath::Fmod(Hours, 24.0f);
    if (TimeOfDay < 0.0f)
    {
        TimeOfDay += 24.0f;
    }
    ApplyBiomeLighting();
}

void ALight_BiomeLightingManager::ApplyBiomeLighting()
{
    FLight_BiomeLightingSettings Settings = GetBiomeSettings(CurrentBiome);
    
    // Apply sun lighting
    if (SunLight && SunLight->GetLightComponent())
    {
        float SunAngle = CalculateSunAngle(TimeOfDay);
        FRotator SunRotation = FRotator(SunAngle, 180.0f, 0.0f);
        SunLight->SetActorRotation(SunRotation);
        
        FLinearColor SunColor = GetSunColorByTime(TimeOfDay);
        float SunIntensity = GetSunIntensityByTime(TimeOfDay) * Settings.SunIntensity;
        
        SunLight->GetLightComponent()->SetLightColor(SunColor);
        SunLight->GetLightComponent()->SetIntensity(SunIntensity);
    }
    
    // Apply sky lighting
    if (SkyLight && SkyLight->GetLightComponent())
    {
        SkyLight->GetLightComponent()->SetLightColor(Settings.SkyLightColor);
        SkyLight->GetLightComponent()->SetIntensity(Settings.SkyLightIntensity);
        SkyLight->GetLightComponent()->RecaptureSky();
    }
    
    // Apply fog
    if (HeightFog && HeightFog->GetComponent())
    {
        HeightFog->GetComponent()->SetFogInscatteringColor(Settings.FogColor);
        HeightFog->GetComponent()->SetFogDensity(Settings.FogDensity);
        HeightFog->GetComponent()->SetStartDistance(Settings.FogStartDistance);
    }
    
    // Apply atmosphere
    if (SkyAtmosphere && SkyAtmosphere->GetSkyAtmosphereComponent())
    {
        // Adjust atmosphere haze based on biome
        SkyAtmosphere->GetSkyAtmosphereComponent()->SetAtmosphereHeight(Settings.AtmosphereHaze * 60.0f);
    }
    
    // Apply clouds
    if (VolumetricClouds && VolumetricClouds->GetVolumetricCloudComponent())
    {
        VolumetricClouds->GetVolumetricCloudComponent()->SetLayerBottomAltitude(1.0f);
        VolumetricClouds->GetVolumetricCloudComponent()->SetLayerHeight(4.0f);
        VolumetricClouds->GetVolumetricCloudComponent()->SetTracingStartMaxDistance(350.0f);
    }
}

void ALight_BiomeLightingManager::UpdateDayNightCycle(float DeltaTime)
{
    float TimeIncrement = (DeltaTime / 60.0f) * (24.0f / DayDurationMinutes);
    TimeOfDay += TimeIncrement;
    
    if (TimeOfDay >= 24.0f)
    {
        TimeOfDay -= 24.0f;
    }
    
    ApplyBiomeLighting();
}

FLight_BiomeLightingSettings ALight_BiomeLightingManager::GetBiomeSettings(ELight_BiomeType Biome)
{
    if (BiomeSettings.Contains(Biome))
    {
        return BiomeSettings[Biome];
    }
    
    // Return default savanna settings if biome not found
    FLight_BiomeLightingSettings DefaultSettings;
    return DefaultSettings;
}

void ALight_BiomeLightingManager::InitializeLightingActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find existing lighting actors
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyLight = Cast<ASkyLight>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        HeightFog = Cast<AExponentialHeightFog>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundActors[0]);
    }
    
    FoundActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    if (FoundActors.Num() > 0)
    {
        VolumetricClouds = Cast<AVolumetricCloud>(FoundActors[0]);
    }
}

void ALight_BiomeLightingManager::SetupBiomePresets()
{
    // Swamp - Dark, humid, greenish tint
    FLight_BiomeLightingSettings SwampSettings;
    SwampSettings.SunColor = FLinearColor(0.8f, 0.9f, 0.7f);
    SwampSettings.SunIntensity = 6.0f;
    SwampSettings.SkyLightColor = FLinearColor(0.4f, 0.6f, 0.5f);
    SwampSettings.SkyLightIntensity = 0.8f;
    SwampSettings.FogColor = FLinearColor(0.6f, 0.8f, 0.6f);
    SwampSettings.FogDensity = 0.08f;
    SwampSettings.FogStartDistance = 500.0f;
    SwampSettings.AtmosphereHaze = 2.0f;
    SwampSettings.CloudCoverage = 0.7f;
    BiomeSettings.Add(ELight_BiomeType::Swamp, SwampSettings);
    
    // Forest - Filtered light, cool tones
    FLight_BiomeLightingSettings ForestSettings;
    ForestSettings.SunColor = FLinearColor(0.9f, 0.95f, 0.8f);
    ForestSettings.SunIntensity = 8.0f;
    ForestSettings.SkyLightColor = FLinearColor(0.3f, 0.5f, 0.7f);
    ForestSettings.SkyLightIntensity = 1.2f;
    ForestSettings.FogColor = FLinearColor(0.7f, 0.8f, 0.9f);
    ForestSettings.FogDensity = 0.04f;
    ForestSettings.FogStartDistance = 800.0f;
    ForestSettings.AtmosphereHaze = 1.5f;
    ForestSettings.CloudCoverage = 0.5f;
    BiomeSettings.Add(ELight_BiomeType::Forest, ForestSettings);
    
    // Savanna - Bright, warm, golden
    FLight_BiomeLightingSettings SavannaSettings;
    SavannaSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.8f);
    SavannaSettings.SunIntensity = 12.0f;
    SavannaSettings.SkyLightColor = FLinearColor(0.6f, 0.7f, 1.0f);
    SavannaSettings.SkyLightIntensity = 1.0f;
    SavannaSettings.FogColor = FLinearColor(0.9f, 0.9f, 0.8f);
    SavannaSettings.FogDensity = 0.01f;
    SavannaSettings.FogStartDistance = 2000.0f;
    SavannaSettings.AtmosphereHaze = 0.8f;
    SavannaSettings.CloudCoverage = 0.2f;
    BiomeSettings.Add(ELight_BiomeType::Savanna, SavannaSettings);
    
    // Desert - Harsh, hot, orange tint
    FLight_BiomeLightingSettings DesertSettings;
    DesertSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    DesertSettings.SunIntensity = 15.0f;
    DesertSettings.SkyLightColor = FLinearColor(0.8f, 0.8f, 1.0f);
    DesertSettings.SkyLightIntensity = 0.6f;
    DesertSettings.FogColor = FLinearColor(1.0f, 0.9f, 0.7f);
    DesertSettings.FogDensity = 0.005f;
    DesertSettings.FogStartDistance = 5000.0f;
    DesertSettings.AtmosphereHaze = 0.5f;
    DesertSettings.CloudCoverage = 0.1f;
    BiomeSettings.Add(ELight_BiomeType::Desert, DesertSettings);
    
    // Snowy Mountain - Cold, blue tint, high contrast
    FLight_BiomeLightingSettings MountainSettings;
    MountainSettings.SunColor = FLinearColor(0.9f, 0.95f, 1.0f);
    MountainSettings.SunIntensity = 10.0f;
    MountainSettings.SkyLightColor = FLinearColor(0.7f, 0.8f, 1.0f);
    MountainSettings.SkyLightIntensity = 1.5f;
    MountainSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f);
    MountainSettings.FogDensity = 0.03f;
    MountainSettings.FogStartDistance = 1500.0f;
    MountainSettings.AtmosphereHaze = 1.2f;
    MountainSettings.CloudCoverage = 0.6f;
    BiomeSettings.Add(ELight_BiomeType::SnowyMountain, MountainSettings);
}

float ALight_BiomeLightingManager::CalculateSunAngle(float TimeHours)
{
    // Sun angle from -90 (sunrise) to 90 (sunset)
    // 6 AM = -90, 12 PM = 0, 6 PM = 90, 12 AM = 180 (below horizon)
    float NormalizedTime = (TimeHours - 6.0f) / 12.0f; // 0-1 for day, outside for night
    
    if (NormalizedTime < 0.0f || NormalizedTime > 1.0f)
    {
        // Night time - sun below horizon
        return 180.0f;
    }
    
    return FMath::Lerp(-90.0f, 90.0f, NormalizedTime);
}

FLinearColor ALight_BiomeLightingManager::GetSunColorByTime(float TimeHours)
{
    if (TimeHours < 6.0f || TimeHours > 18.0f)
    {
        // Night - very dim blue
        return FLinearColor(0.1f, 0.15f, 0.3f);
    }
    else if (TimeHours < 8.0f || TimeHours > 16.0f)
    {
        // Dawn/Dusk - orange/red
        return FLinearColor(1.0f, 0.6f, 0.3f);
    }
    else
    {
        // Day - white/yellow
        return FLinearColor(1.0f, 0.95f, 0.8f);
    }
}

float ALight_BiomeLightingManager::GetSunIntensityByTime(float TimeHours)
{
    if (TimeHours < 6.0f || TimeHours > 18.0f)
    {
        // Night
        return 0.1f;
    }
    else if (TimeHours < 8.0f || TimeHours > 16.0f)
    {
        // Dawn/Dusk
        return 0.4f;
    }
    else
    {
        // Day
        return 1.0f;
    }
}