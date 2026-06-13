#include "EnvArt_AtmosphericManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/VolumetricCloud.h"
#include "Components/VolumetricCloudComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AEnvArt_AtmosphericManager::AEnvArt_AtmosphericManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    CurrentTimeOfDay = 12.0f; // Noon
    DayDurationInMinutes = 20.0f; // 20 minute day cycle
    bEnableDayNightCycle = true;
    WeatherIntensity = 0.5f;
    bIsRaining = false;
    bIsFoggy = true;
    CurrentZoneName = TEXT("Default");
    TimeAccumulator = 0.0f;
}

void AEnvArt_AtmosphericManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default atmospheric zones
    InitializeDefaultAtmosphericZones();
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericManager: Initialized with %d zones"), AtmosphericZones.Num());
}

void AEnvArt_AtmosphericManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bEnableDayNightCycle)
    {
        TimeAccumulator += DeltaTime;
        
        // Update time of day
        float DayDurationInSeconds = DayDurationInMinutes * 60.0f;
        CurrentTimeOfDay = FMath::Fmod(TimeAccumulator / DayDurationInSeconds * 24.0f, 24.0f);
        
        // Update lighting based on time
        UpdateLightingForTimeOfDay(CurrentTimeOfDay);
    }
    
    // Update atmospheric effects
    UpdateAtmosphericEffects(DeltaTime);
}

void AEnvArt_AtmosphericManager::SetAtmosphericZone(const FString& ZoneName, const FVector& PlayerLocation)
{
    if (CurrentZoneName != ZoneName)
    {
        CurrentZoneName = ZoneName;
        
        // Find the zone and apply its settings
        for (const FEnvArt_AtmosphericZone& Zone : AtmosphericZones)
        {
            if (Zone.ZoneName == ZoneName)
            {
                // Apply zone-specific atmospheric settings
                UpdateDirectionalLight();
                UpdateVolumetricFog();
                
                UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Switched to zone %s"), *ZoneName);
                break;
            }
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateLightingForTimeOfDay(float TimeOfDay)
{
    // Calculate sun angle based on time of day
    float SunAngle = (TimeOfDay / 24.0f) * 360.0f - 90.0f; // -90 to 270 degrees
    
    // Find directional light in the world
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
        {
            // Update sun rotation
            FRotator NewRotation = FRotator(SunAngle, 30.0f, 0.0f);
            DirLight->SetActorRotation(NewRotation);
            
            // Update light color and intensity based on time
            UDirectionalLightComponent* LightComp = DirLight->GetComponent<UDirectionalLightComponent>();
            if (LightComp)
            {
                float IntensityMultiplier = 1.0f;
                FLinearColor LightColor = FLinearColor::White;
                
                // Dawn/Dusk (5-7 AM, 6-8 PM)
                if ((TimeOfDay >= 5.0f && TimeOfDay <= 7.0f) || (TimeOfDay >= 18.0f && TimeOfDay <= 20.0f))
                {
                    LightColor = FLinearColor(1.0f, 0.7f, 0.4f, 1.0f); // Orange
                    IntensityMultiplier = 0.6f;
                }
                // Day (7 AM - 6 PM)
                else if (TimeOfDay >= 7.0f && TimeOfDay <= 18.0f)
                {
                    LightColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f); // Warm white
                    IntensityMultiplier = 1.0f;
                }
                // Night (8 PM - 5 AM)
                else
                {
                    LightColor = FLinearColor(0.3f, 0.4f, 0.8f, 1.0f); // Blue moonlight
                    IntensityMultiplier = 0.1f;
                }
                
                LightComp->SetLightColor(LightColor);
                LightComp->SetIntensity(8.0f * IntensityMultiplier);
            }
        }
    }
}

void AEnvArt_AtmosphericManager::SpawnParticleEffect(const FString& EffectName, const FVector& Location)
{
    // Add particle effect to active list
    FEnvArt_ParticleEffect NewEffect;
    NewEffect.EffectName = EffectName;
    NewEffect.SpawnLocation = Location;
    NewEffect.bIsActive = true;
    
    ParticleEffects.Add(NewEffect);
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Spawned particle effect %s at location %s"), 
           *EffectName, *Location.ToString());
}

void AEnvArt_AtmosphericManager::SetWeatherConditions(bool bRain, bool bFog, float Intensity)
{
    bIsRaining = bRain;
    bIsFoggy = bFog;
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    // Update atmospheric effects based on weather
    UpdateVolumetricFog();
    
    UE_LOG(LogTemp, Log, TEXT("AtmosphericManager: Weather updated - Rain: %s, Fog: %s, Intensity: %.2f"),
           bIsRaining ? TEXT("true") : TEXT("false"),
           bIsFoggy ? TEXT("true") : TEXT("false"),
           WeatherIntensity);
}

FEnvArt_AtmosphericZone AEnvArt_AtmosphericManager::GetCurrentAtmosphericZone(const FVector& Location) const
{
    // Find the closest atmospheric zone
    float MinDistance = FLT_MAX;
    FEnvArt_AtmosphericZone ClosestZone;
    
    for (const FEnvArt_AtmosphericZone& Zone : AtmosphericZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance < Zone.ZoneRadius && Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestZone = Zone;
        }
    }
    
    return ClosestZone;
}

void AEnvArt_AtmosphericManager::InitializeDefaultAtmosphericZones()
{
    AtmosphericZones.Empty();
    
    // Forest Zone
    FEnvArt_AtmosphericZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest");
    ForestZone.ZoneCenter = FVector(0, 0, 0);
    ForestZone.ZoneRadius = 3000.0f;
    ForestZone.FogColor = FLinearColor(0.8f, 0.9f, 0.7f, 1.0f);
    ForestZone.FogDensity = 0.15f;
    ForestZone.LightColor = FLinearColor(0.9f, 0.95f, 0.7f, 1.0f);
    ForestZone.LightIntensity = 6.0f;
    AtmosphericZones.Add(ForestZone);
    
    // Plains Zone
    FEnvArt_AtmosphericZone PlainsZone;
    PlainsZone.ZoneName = TEXT("Plains");
    PlainsZone.ZoneCenter = FVector(5000, 5000, 0);
    PlainsZone.ZoneRadius = 4000.0f;
    PlainsZone.FogColor = FLinearColor(0.95f, 0.95f, 0.9f, 1.0f);
    PlainsZone.FogDensity = 0.05f;
    PlainsZone.LightColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
    PlainsZone.LightIntensity = 10.0f;
    AtmosphericZones.Add(PlainsZone);
    
    // River Zone
    FEnvArt_AtmosphericZone RiverZone;
    RiverZone.ZoneName = TEXT("River");
    RiverZone.ZoneCenter = FVector(-2000, 3000, 0);
    RiverZone.ZoneRadius = 1500.0f;
    RiverZone.FogColor = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);
    RiverZone.FogDensity = 0.2f;
    RiverZone.LightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    RiverZone.LightIntensity = 7.0f;
    AtmosphericZones.Add(RiverZone);
    
    UE_LOG(LogTemp, Warning, TEXT("AtmosphericManager: Initialized %d default atmospheric zones"), AtmosphericZones.Num());
}

void AEnvArt_AtmosphericManager::UpdateAtmosphericEffects(float DeltaTime)
{
    // Update particle effects
    for (int32 i = ParticleEffects.Num() - 1; i >= 0; i--)
    {
        FEnvArt_ParticleEffect& Effect = ParticleEffects[i];
        
        if (Effect.bIsActive)
        {
            Effect.ParticleLifetime -= DeltaTime;
            
            if (Effect.ParticleLifetime <= 0.0f)
            {
                Effect.bIsActive = false;
                ParticleEffects.RemoveAt(i);
            }
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateDirectionalLight()
{
    // Implementation for updating directional light based on current zone
    UWorld* World = GetWorld();
    if (!World) return;
    
    FEnvArt_AtmosphericZone CurrentZone = GetCurrentAtmosphericZone(GetActorLocation());
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (ADirectionalLight* DirLight = Cast<ADirectionalLight>(Actor))
        {
            UDirectionalLightComponent* LightComp = DirLight->GetComponent<UDirectionalLightComponent>();
            if (LightComp)
            {
                LightComp->SetLightColor(CurrentZone.LightColor);
                LightComp->SetIntensity(CurrentZone.LightIntensity);
            }
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateVolumetricFog()
{
    // Implementation for updating volumetric fog based on current zone and weather
    UWorld* World = GetWorld();
    if (!World) return;
    
    FEnvArt_AtmosphericZone CurrentZone = GetCurrentAtmosphericZone(GetActorLocation());
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVolumetricCloud::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (AVolumetricCloud* CloudActor = Cast<AVolumetricCloud>(Actor))
        {
            UVolumetricCloudComponent* CloudComp = CloudActor->GetVolumetricCloudComponent();
            if (CloudComp)
            {
                float FinalDensity = CurrentZone.FogDensity;
                if (bIsFoggy)
                {
                    FinalDensity *= (1.0f + WeatherIntensity);
                }
                
                CloudComp->SetAlbedo(CurrentZone.FogColor);
                // Note: Density setting would depend on specific UE5 volumetric cloud properties
            }
        }
    }
}

void AEnvArt_AtmosphericManager::UpdateParticleEffects()
{
    // Implementation for updating active particle effects
    for (FEnvArt_ParticleEffect& Effect : ParticleEffects)
    {
        if (Effect.bIsActive)
        {
            // Update particle system properties based on weather and zone
            float IntensityMultiplier = 1.0f;
            
            if (bIsRaining)
            {
                IntensityMultiplier *= (1.0f + WeatherIntensity * 0.5f);
            }
            
            Effect.SpawnRate *= IntensityMultiplier;
        }
    }
}

void AEnvArt_AtmosphericManager::CalculateAtmosphericBlending(const FVector& PlayerLocation)
{
    // Calculate blending between multiple atmospheric zones
    TArray<float> ZoneInfluences;
    float TotalInfluence = 0.0f;
    
    for (const FEnvArt_AtmosphericZone& Zone : AtmosphericZones)
    {
        float Distance = FVector::Dist(PlayerLocation, Zone.ZoneCenter);
        float Influence = FMath::Max(0.0f, 1.0f - (Distance / Zone.ZoneRadius));
        ZoneInfluences.Add(Influence);
        TotalInfluence += Influence;
    }
    
    // Normalize influences and apply blended atmospheric settings
    if (TotalInfluence > 0.0f)
    {
        for (int32 i = 0; i < ZoneInfluences.Num(); i++)
        {
            ZoneInfluences[i] /= TotalInfluence;
        }
    }
}