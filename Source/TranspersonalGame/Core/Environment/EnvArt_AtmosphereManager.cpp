#include "EnvArt_AtmosphereManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

AEnvArt_AtmosphereManager::AEnvArt_AtmosphereManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    AtmosphereSettings.SunIntensity = 8.0f;
    AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    AtmosphereSettings.SunElevation = -45.0f;
    AtmosphereSettings.SunAzimuth = 225.0f;
    AtmosphereSettings.FogDensity = 0.02f;
    AtmosphereSettings.FogHeightFalloff = 0.2f;
    AtmosphereSettings.FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    AtmosphereSettings.bEnableAtmosphericParticles = true;
    AtmosphereSettings.ParticleDensity = 1.0f;
}

void AEnvArt_AtmosphereManager::BeginPlay()
{
    Super::BeginPlay();
    
    FindLightingActors();
    UpdateAtmosphere();
    bIsInitialized = true;
}

void AEnvArt_AtmosphereManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsInitialized)
    {
        // Update atmospheric effects based on time or weather changes
        UpdateAtmosphericParticles();
    }
}

void AEnvArt_AtmosphereManager::FindLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find main directional light
    for (TActorIterator<ADirectionalLight> ActorItr(World); ActorItr; ++ActorItr)
    {
        ADirectionalLight* DirectionalLight = *ActorItr;
        if (DirectionalLight && DirectionalLight->GetName().Contains(TEXT("DirectionalLight")))
        {
            MainSunLight = DirectionalLight;
            break;
        }
    }

    // Find fog actors
    FogActors.Empty();
    for (TActorIterator<AExponentialHeightFog> ActorItr(World); ActorItr; ++ActorItr)
    {
        AExponentialHeightFog* FogActor = *ActorItr;
        if (FogActor)
        {
            FogActors.Add(FogActor);
        }
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphere()
{
    UpdateSunLighting();
    UpdateVolumetricFog();
    OnAtmosphereUpdated();
}

void AEnvArt_AtmosphereManager::UpdateSunLighting()
{
    if (!MainSunLight)
    {
        return;
    }

    // Set sun rotation based on time of day
    FRotator SunRotation;
    SunRotation.Pitch = AtmosphereSettings.SunElevation;
    SunRotation.Yaw = AtmosphereSettings.SunAzimuth;
    SunRotation.Roll = 0.0f;
    
    MainSunLight->SetActorRotation(SunRotation);

    // Update light component properties
    UDirectionalLightComponent* LightComp = MainSunLight->GetLightComponent();
    if (LightComp)
    {
        LightComp->SetIntensity(AtmosphereSettings.SunIntensity);
        LightComp->SetLightColor(AtmosphereSettings.SunColor);
        LightComp->SetCastVolumetricShadow(true);
        LightComp->SetCastShadows(true);
    }
}

void AEnvArt_AtmosphereManager::UpdateVolumetricFog()
{
    for (AExponentialHeightFog* FogActor : FogActors)
    {
        if (FogActor)
        {
            UExponentialHeightFogComponent* FogComp = FogActor->GetComponent();
            if (FogComp)
            {
                FogComp->SetFogDensity(AtmosphereSettings.FogDensity);
                FogComp->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
                FogComp->SetFogInscatteringColor(AtmosphereSettings.FogColor);
                FogComp->SetVolumetricFog(true);
                FogComp->SetVolumetricFogScatteringDistribution(0.6f);
            }
        }
    }
}

void AEnvArt_AtmosphereManager::UpdateAtmosphericParticles()
{
    // Update particle systems based on current settings
    // This would integrate with Niagara particle systems
    for (AActor* ParticleActor : ParticleActors)
    {
        if (ParticleActor && AtmosphereSettings.bEnableAtmosphericParticles)
        {
            // Enable/update particle effects
            ParticleActor->SetActorHiddenInGame(false);
        }
        else if (ParticleActor)
        {
            // Disable particle effects
            ParticleActor->SetActorHiddenInGame(true);
        }
    }
}

void AEnvArt_AtmosphereManager::SetTimeOfDay(float TimeHours)
{
    CurrentTimeOfDay = FMath::Clamp(TimeHours, 0.0f, 24.0f);
    
    // Calculate sun position based on time
    float SunAngle = (CurrentTimeOfDay - 12.0f) * 15.0f; // 15 degrees per hour
    AtmosphereSettings.SunElevation = FMath::Sin(FMath::DegreesToRadians(SunAngle)) * 90.0f;
    AtmosphereSettings.SunAzimuth = 180.0f + SunAngle;
    
    // Adjust sun color based on time
    if (CurrentTimeOfDay >= 6.0f && CurrentTimeOfDay <= 8.0f) // Dawn
    {
        AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);
        AtmosphereSettings.SunIntensity = 6.0f;
    }
    else if (CurrentTimeOfDay >= 17.0f && CurrentTimeOfDay <= 19.0f) // Dusk
    {
        AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.7f, 0.5f, 1.0f);
        AtmosphereSettings.SunIntensity = 5.0f;
    }
    else if (CurrentTimeOfDay >= 10.0f && CurrentTimeOfDay <= 16.0f) // Midday
    {
        AtmosphereSettings.SunColor = FLinearColor(1.0f, 0.95f, 0.9f, 1.0f);
        AtmosphereSettings.SunIntensity = 10.0f;
    }
    else // Night
    {
        AtmosphereSettings.SunColor = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);
        AtmosphereSettings.SunIntensity = 1.0f;
    }
    
    UpdateAtmosphere();
}

void AEnvArt_AtmosphereManager::CreateVolumetricFogAt(FVector Location, float Density)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AExponentialHeightFog* NewFog = World->SpawnActor<AExponentialHeightFog>(Location, FRotator::ZeroRotator);
    if (NewFog)
    {
        UExponentialHeightFogComponent* FogComp = NewFog->GetComponent();
        if (FogComp)
        {
            FogComp->SetFogDensity(Density);
            FogComp->SetFogHeightFalloff(AtmosphereSettings.FogHeightFalloff);
            FogComp->SetFogInscatteringColor(AtmosphereSettings.FogColor);
            FogComp->SetVolumetricFog(true);
        }
        
        FogActors.Add(NewFog);
    }
}

void AEnvArt_AtmosphereManager::SpawnAtmosphericParticlesAt(FVector Location)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Create placeholder actor for particle system
    AActor* ParticleActor = World->SpawnActor<AActor>(Location, FRotator::ZeroRotator);
    if (ParticleActor)
    {
        ParticleActor->SetActorLabel(TEXT("AtmosphericParticles"));
        ParticleActors.Add(ParticleActor);
    }
}

void AEnvArt_AtmosphereManager::SetWeatherCondition(EWeatherType WeatherType)
{
    switch (WeatherType)
    {
        case EWeatherType::Clear:
            AtmosphereSettings.FogDensity = 0.01f;
            AtmosphereSettings.SunIntensity = 10.0f;
            break;
            
        case EWeatherType::Foggy:
            AtmosphereSettings.FogDensity = 0.08f;
            AtmosphereSettings.SunIntensity = 4.0f;
            break;
            
        case EWeatherType::Overcast:
            AtmosphereSettings.FogDensity = 0.03f;
            AtmosphereSettings.SunIntensity = 6.0f;
            AtmosphereSettings.SunColor = FLinearColor(0.8f, 0.8f, 0.9f, 1.0f);
            break;
            
        case EWeatherType::Stormy:
            AtmosphereSettings.FogDensity = 0.05f;
            AtmosphereSettings.SunIntensity = 3.0f;
            AtmosphereSettings.SunColor = FLinearColor(0.6f, 0.6f, 0.7f, 1.0f);
            break;
    }
    
    UpdateAtmosphere();
}