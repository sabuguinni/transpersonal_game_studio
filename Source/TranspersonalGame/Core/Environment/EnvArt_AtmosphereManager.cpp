#include "EnvArt_AtmosphereManager.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/VolumetricFog.h"
#include "Engine/SkyAtmosphere.h"
#include "Kismet/GameplayStatics.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize default settings
    TimeOfDaySettings.SunElevation = -15.0f;
    TimeOfDaySettings.SunAzimuth = 45.0f;
    TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    TimeOfDaySettings.SunIntensity = 3.5f;
    TimeOfDaySettings.bCastVolumetricShadows = true;

    FogSettings.ScatteringDistribution = 0.2f;
    FogSettings.FogAlbedo = FColor(180, 200, 160);
    FogSettings.ExtinctionScale = 0.5f;
    FogSettings.ViewDistance = 50000.0f;

    AtmosphereSettings.RayleighScatteringScale = 0.8f;
    AtmosphereSettings.MieScatteringScale = 0.6f;
    AtmosphereSettings.MultiScatteringFactor = 1.2f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindSceneReferences();
    ApplyGoldenHourLighting();
    CreateForestFog();
    EnhanceAtmosphere();
    CreateAtmosphericParticles();
}

void AEnvArt_AtmosphereManager::FindSceneReferences()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find directional light (sun)
    TArray<AActor*> FoundLights;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundLights);
    if (FoundLights.Num() > 0)
    {
        SunLight = Cast<ADirectionalLight>(FoundLights[0]);
    }

    // Find volumetric fog
    TArray<AActor*> FoundFogs;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricFog::StaticClass(), FoundFogs);
    if (FoundFogs.Num() > 0)
    {
        ForestFog = Cast<AVolumetricFog>(FoundFogs[0]);
    }

    // Find sky atmosphere
    TArray<AActor*> FoundAtmospheres;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), FoundAtmospheres);
    if (FoundAtmospheres.Num() > 0)
    {
        SkyAtmosphere = Cast<ASkyAtmosphere>(FoundAtmospheres[0]);
    }
}

void AEnvArt_AtmosphereManager::ApplyGoldenHourLighting()
{
    if (!SunLight) return;

    // Set sun rotation for golden hour
    FRotator SunRotation(TimeOfDaySettings.SunElevation, TimeOfDaySettings.SunAzimuth, 0.0f);
    SunLight->SetActorRotation(SunRotation);

    // Configure directional light component
    UDirectionalLightComponent* LightComp = SunLight->GetDirectionalLightComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(TimeOfDaySettings.SunIntensity);
        LightComp->SetLightColor(TimeOfDaySettings.SunColor);
        LightComp->SetCastVolumetricShadow(TimeOfDaySettings.bCastVolumetricShadows);
        LightComp->SetVolumetricScatteringIntensity(2.0f);
        LightComp->SetAtmosphereSunLight(true);
    }
}

void AEnvArt_AtmosphereManager::CreateForestFog()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (!ForestFog)
    {
        // Spawn volumetric fog in forest biome
        FVector FogLocation(25000.0f, 25000.0f, 500.0f);
        ForestFog = World->SpawnActor<AVolumetricFog>(AVolumetricFog::StaticClass(), FogLocation, FRotator::ZeroRotator);
    }

    if (ForestFog)
    {
        UVolumetricFogComponent* FogComp = ForestFog->GetVolumetricFogComponent();
        if (FogComp)
        {
            FogComp->SetScatteringDistribution(FogSettings.ScatteringDistribution);
            FogComp->SetAlbedo(FogSettings.FogAlbedo);
            FogComp->SetExtinctionScale(FogSettings.ExtinctionScale);
            FogComp->SetViewDistance(FogSettings.ViewDistance);
        }
    }
}

void AEnvArt_AtmosphereManager::EnhanceAtmosphere()
{
    if (!SkyAtmosphere) return;

    USkyAtmosphereComponent* AtmoComp = SkyAtmosphere->GetAtmosphereComponent();
    if (AtmoComp)
    {
        AtmoComp->SetRayleighScatteringScale(AtmosphereSettings.RayleighScatteringScale);
        AtmoComp->SetMieScatteringScale(AtmosphereSettings.MieScatteringScale);
        AtmoComp->SetMultiScatteringFactor(AtmosphereSettings.MultiScatteringFactor);
    }
}

void AEnvArt_AtmosphereManager::CreateAtmosphericParticles()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Create dust/pollen particle emitters in forest areas
    for (int32 i = 0; i < 3; i++)
    {
        FVector ParticleLocation(20000.0f + i * 10000.0f, 20000.0f + i * 8000.0f, 1000.0f);
        
        // Note: In a real implementation, you would spawn particle system actors here
        // For now, we create placeholder actors that can be configured with Niagara systems
        AActor* ParticleActor = World->SpawnActor<AActor>(AActor::StaticClass(), ParticleLocation, FRotator::ZeroRotator);
        if (ParticleActor)
        {
            ParticleActor->SetActorLabel(FString::Printf(TEXT("AtmosphericDust_%d"), i + 1));
        }
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float Hours24Format)
{
    CurrentTimeOfDay = FMath::Clamp(Hours24Format, 0.0f, 24.0f);
    
    // Convert to sun elevation and azimuth
    float NormalizedTime = CurrentTimeOfDay / 24.0f;
    TimeOfDaySettings.SunElevation = FMath::Sin(NormalizedTime * PI) * 90.0f - 90.0f; // -90 to 0 degrees
    TimeOfDaySettings.SunAzimuth = NormalizedTime * 360.0f; // 0 to 360 degrees
    
    // Adjust sun color based on time
    if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay <= 18.0f) // Golden hour
    {
        TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        TimeOfDaySettings.SunIntensity = 3.5f;
    }
    else if (CurrentTimeOfDay >= 12.0f && CurrentTimeOfDay < 16.0f) // Midday
    {
        TimeOfDaySettings.SunColor = FLinearColor(1.0f, 1.0f, 0.95f, 1.0f);
        TimeOfDaySettings.SunIntensity = 5.0f;
    }
    else // Dawn/Dusk/Night
    {
        TimeOfDaySettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        TimeOfDaySettings.SunIntensity = 2.0f;
    }
    
    UpdateSunPosition();
    UpdateFogDensity();
    UpdateAtmosphericScattering();
}

void AEnvArt_AtmosphereManager::UpdateSunPosition()
{
    if (SunLight)
    {
        FRotator NewRotation(TimeOfDaySettings.SunElevation, TimeOfDaySettings.SunAzimuth, 0.0f);
        SunLight->SetActorRotation(NewRotation);
        
        UDirectionalLightComponent* LightComp = SunLight->GetDirectionalLightComponent();
        if (LightComp)
        {
            LightComp->SetIntensity(TimeOfDaySettings.SunIntensity);
            LightComp->SetLightColor(TimeOfDaySettings.SunColor);
        }
    }
}

void AEnvArt_AtmosphereManager::UpdateFogDensity()
{
    if (ForestFog)
    {
        UVolumetricFogComponent* FogComp = ForestFog->GetVolumetricFogComponent();
        if (FogComp)
        {
            // Increase fog density during dawn/dusk
            float TimeBasedDensity = FogSettings.ExtinctionScale;
            if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 8.0f || 
                CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay <= 19.0f)
            {
                TimeBasedDensity *= 1.5f;
            }
            FogComp->SetExtinctionScale(TimeBasedDensity);
        }
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphericScattering()
{
    if (SkyAtmosphere)
    {
        USkyAtmosphereComponent* AtmoComp = SkyAtmosphere->GetAtmosphereComponent();
        if (AtmoComp)
        {
            // Enhance scattering during golden hour
            float ScatteringMultiplier = 1.0f;
            if (CurrentTimeOfDay >= 16.0f && CurrentTimeOfDay <= 18.0f)
            {
                ScatteringMultiplier = 1.3f;
            }
            
            AtmoComp->SetRayleighScatteringScale(AtmosphereSettings.RayleighScatteringScale * ScatteringMultiplier);
            AtmoComp->SetMieScatteringScale(AtmosphereSettings.MieScatteringScale * ScatteringMultiplier);
        }
    }
}