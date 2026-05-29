#include "VFX_EnvironmentalEffectManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/PointLight.h"

AVFX_EnvironmentalEffectManager::AVFX_EnvironmentalEffectManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default campfire locations
    CampfireLocations.Add(FVector(0.0f, 0.0f, 100.0f));           // Central camp
    CampfireLocations.Add(FVector(-44000.0f, 39000.0f, 100.0f));  // Forest camp
    CampfireLocations.Add(FVector(49000.0f, -39000.0f, 100.0f));  // Desert camp

    // Performance settings
    MaxActiveEffects = 20;
    EffectUpdateInterval = 0.1f;
    LODDistanceNear = 1000.0f;
    LODDistanceFar = 5000.0f;
    bEnableLODSystem = true;

    // Atmospheric settings
    ForestFogDensity = 0.3f;
    DesertHazeDensity = 0.2f;
    WindParticleIntensity = 1.0f;

    // Campfire settings
    CampfireLightIntensity = 2000.0f;
    CampfireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
}

void AVFX_EnvironmentalEffectManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeCampfires();
    
    // Create initial atmospheric effects
    CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType::ForestFog, 
        FVector(-45000.0f, 40000.0f, 200.0f), ForestFogDensity);
    
    CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType::DesertHaze, 
        FVector(50000.0f, -40000.0f, 150.0f), DesertHazeDensity);

    UE_LOG(LogTemp, Warning, TEXT("VFX Environmental Effect Manager initialized with %d campfires"), CampfireLocations.Num());
}

void AVFX_EnvironmentalEffectManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= EffectUpdateInterval)
    {
        UpdateEnvironmentalEffects(DeltaTime);
        LastUpdateTime = 0.0f;
    }
    
    // Performance optimization
    if (bEnableLODSystem)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            OptimizeEffectsForDistance(PlayerPawn->GetActorLocation());
        }
    }
}

void AVFX_EnvironmentalEffectManager::CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType EffectType, FVector Location, float Intensity, float Duration)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupInactiveEffects();
        
        if (ActiveEffects.Num() >= MaxActiveEffects)
        {
            UE_LOG(LogTemp, Warning, TEXT("VFX: Maximum active effects reached, cannot create new effect"));
            return;
        }
    }

    FVFX_EnvironmentalEffect NewEffect;
    NewEffect.EffectType = EffectType;
    NewEffect.Location = Location;
    NewEffect.Intensity = Intensity;
    NewEffect.Duration = Duration;
    NewEffect.bIsActive = true;

    ActiveEffects.Add(NewEffect);

    UE_LOG(LogTemp, Log, TEXT("VFX: Created environmental effect type %d at location %s"), 
        (int32)EffectType, *Location.ToString());
}

void AVFX_EnvironmentalEffectManager::RemoveEnvironmentalEffect(int32 EffectIndex)
{
    if (EffectIndex >= 0 && EffectIndex < ActiveEffects.Num())
    {
        ActiveEffects[EffectIndex].bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("VFX: Deactivated environmental effect at index %d"), EffectIndex);
    }
}

void AVFX_EnvironmentalEffectManager::UpdateEffectIntensity(int32 EffectIndex, float NewIntensity)
{
    if (EffectIndex >= 0 && EffectIndex < ActiveEffects.Num())
    {
        ActiveEffects[EffectIndex].Intensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
        UE_LOG(LogTemp, Log, TEXT("VFX: Updated effect intensity to %f"), NewIntensity);
    }
}

void AVFX_EnvironmentalEffectManager::CreateCampfire(FVector Location)
{
    // Create campfire particle effect
    CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType::CampfireFire, Location, 1.0f, -1.0f);
    
    // Create point light for illumination
    UWorld* World = GetWorld();
    if (World)
    {
        FVector LightLocation = Location + FVector(0.0f, 0.0f, 50.0f);
        APointLight* CampfireLight = World->SpawnActor<APointLight>(APointLight::StaticClass(), LightLocation, FRotator::ZeroRotator);
        
        if (CampfireLight)
        {
            UPointLightComponent* LightComp = CampfireLight->GetPointLightComponent();
            if (LightComp)
            {
                LightComp->SetLightColor(CampfireLightColor);
                LightComp->SetIntensity(CampfireLightIntensity);
                LightComp->SetAttenuationRadius(800.0f);
                LightComp->SetSourceRadius(20.0f);
                LightComp->SetSoftSourceRadius(40.0f);
            }
            
            CampfireLight->SetActorLabel(FString::Printf(TEXT("CampfireLight_%d"), CampfireLocations.Num()));
            LightComponents.Add(LightComp);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VFX: Created campfire at location %s"), *Location.ToString());
}

void AVFX_EnvironmentalEffectManager::ExtinguishCampfire(FVector Location, float SearchRadius)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (ActiveEffects[i].EffectType == EVFX_EnvironmentalEffectType::CampfireFire)
        {
            float Distance = FVector::Dist(ActiveEffects[i].Location, Location);
            if (Distance <= SearchRadius)
            {
                RemoveEnvironmentalEffect(i);
                UE_LOG(LogTemp, Log, TEXT("VFX: Extinguished campfire at distance %f"), Distance);
                break;
            }
        }
    }
}

void AVFX_EnvironmentalEffectManager::CreateFootstepImpact(FVector Location, float ImpactForce)
{
    float EffectIntensity = FMath::Clamp(ImpactForce, 0.1f, 2.0f);
    CreateEnvironmentalEffect(EVFX_EnvironmentalEffectType::FootstepDust, Location, EffectIntensity, 2.0f);
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Created footstep impact at %s with force %f"), *Location.ToString(), ImpactForce);
}

void AVFX_EnvironmentalEffectManager::SetWeatherIntensity(float RainIntensity, float FogIntensity, float WindIntensity)
{
    // Update existing atmospheric effects
    for (FVFX_EnvironmentalEffect& Effect : ActiveEffects)
    {
        switch (Effect.EffectType)
        {
            case EVFX_EnvironmentalEffectType::RainDroplets:
                Effect.Intensity = FMath::Clamp(RainIntensity, 0.0f, 2.0f);
                break;
            case EVFX_EnvironmentalEffectType::ForestFog:
            case EVFX_EnvironmentalEffectType::DesertHaze:
                Effect.Intensity = FMath::Clamp(FogIntensity, 0.0f, 2.0f);
                break;
            case EVFX_EnvironmentalEffectType::WindParticles:
                Effect.Intensity = FMath::Clamp(WindIntensity, 0.0f, 2.0f);
                break;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("VFX: Updated weather intensities - Rain: %f, Fog: %f, Wind: %f"), 
        RainIntensity, FogIntensity, WindIntensity);
}

void AVFX_EnvironmentalEffectManager::OptimizeEffectsForDistance(FVector PlayerLocation)
{
    for (int32 i = 0; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].bIsActive)
        {
            float LODLevel = CalculateLODLevel(ActiveEffects[i].Location, PlayerLocation);
            ApplyLODToEffect(i, LODLevel);
        }
    }
}

void AVFX_EnvironmentalEffectManager::CleanupInactiveEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        if (!ActiveEffects[i].bIsActive)
        {
            ActiveEffects.RemoveAt(i);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX: Cleaned up inactive effects, %d effects remaining"), ActiveEffects.Num());
}

void AVFX_EnvironmentalEffectManager::InitializeCampfires()
{
    for (const FVector& Location : CampfireLocations)
    {
        CreateCampfire(Location);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX: Initialized %d campfires"), CampfireLocations.Num());
}

void AVFX_EnvironmentalEffectManager::UpdateEnvironmentalEffects(float DeltaTime)
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FVFX_EnvironmentalEffect& Effect = ActiveEffects[i];
        
        if (!Effect.bIsActive)
            continue;
            
        // Update duration-based effects
        if (Effect.Duration > 0.0f)
        {
            Effect.Duration -= DeltaTime;
            if (Effect.Duration <= 0.0f)
            {
                Effect.bIsActive = false;
            }
        }
    }
}

float AVFX_EnvironmentalEffectManager::CalculateLODLevel(FVector EffectLocation, FVector PlayerLocation)
{
    float Distance = FVector::Dist(EffectLocation, PlayerLocation);
    
    if (Distance <= LODDistanceNear)
    {
        return 1.0f; // High quality
    }
    else if (Distance >= LODDistanceFar)
    {
        return 0.0f; // Disabled
    }
    else
    {
        // Linear interpolation between near and far distances
        float Alpha = (Distance - LODDistanceNear) / (LODDistanceFar - LODDistanceNear);
        return FMath::Lerp(1.0f, 0.0f, Alpha);
    }
}

void AVFX_EnvironmentalEffectManager::ApplyLODToEffect(int32 EffectIndex, float LODLevel)
{
    if (EffectIndex >= 0 && EffectIndex < ActiveEffects.Num())
    {
        FVFX_EnvironmentalEffect& Effect = ActiveEffects[EffectIndex];
        
        // Adjust effect intensity based on LOD level
        float AdjustedIntensity = Effect.Intensity * LODLevel;
        
        // Disable effect if LOD level is too low
        if (LODLevel < 0.1f)
        {
            Effect.bIsActive = false;
        }
    }
}