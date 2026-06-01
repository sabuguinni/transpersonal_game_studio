#include "VFX_EffectsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AVFX_EffectsManager::AVFX_EffectsManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    MaxActiveEffects = 100;
    EffectCullDistance = 10000.0f;
    bEnableWeatherEffects = true;
    bEnableDinosaurVFX = true;
    bEnableAmbientVFX = true;
    
    EffectCleanupTimer = 0.0f;
    LODUpdateTimer = 0.0f;
}

void AVFX_EffectsManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("VFX Effects Manager initialized"));
    
    // Clear any existing effects
    ActiveEffects.Empty();
}

void AVFX_EffectsManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update timers
    EffectCleanupTimer += DeltaTime;
    LODUpdateTimer += DeltaTime;

    // Cleanup expired effects every 2 seconds
    if (EffectCleanupTimer >= 2.0f)
    {
        CleanupExpiredEffects();
        EffectCleanupTimer = 0.0f;
    }

    // Update LOD every 1 second
    if (LODUpdateTimer >= 1.0f)
    {
        UpdateEffectLOD();
        LODUpdateTimer = 0.0f;
    }
}

void AVFX_EffectsManager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FVector Scale, float Duration)
{
    // Check if we're at the effect limit
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        // Remove oldest effect
        ActiveEffects.RemoveAt(0);
    }

    // Create new effect data
    FVFX_EffectData NewEffect;
    NewEffect.EffectType = EffectType;
    NewEffect.Location = Location;
    NewEffect.Scale = Scale;
    NewEffect.Duration = Duration;
    NewEffect.Intensity = 1.0f;

    // Add to active effects
    ActiveEffects.Add(NewEffect);

    // Log effect creation
    UE_LOG(LogTemp, Log, TEXT("VFX Effect spawned: Type=%d at Location=(%f,%f,%f)"), 
           (int32)EffectType, Location.X, Location.Y, Location.Z);
}

void AVFX_EffectsManager::SpawnDinosaurFootstepVFX(FVector Location, float DinosaurSize)
{
    if (!bEnableDinosaurVFX) return;

    FVector Scale = FVector(DinosaurSize * 2.0f, DinosaurSize * 2.0f, DinosaurSize);
    SpawnEffect(EVFX_EffectType::DinosaurFootstep, Location, Scale, 3.0f);
}

void AVFX_EffectsManager::SpawnCampfireVFX(FVector Location)
{
    SpawnEffect(EVFX_EffectType::Campfire, Location, FVector::OneVector, 0.0f); // Permanent fire
}

void AVFX_EffectsManager::SpawnWeatherVFX(EVFX_EffectType WeatherType, FVector Location, float Intensity)
{
    if (!bEnableWeatherEffects) return;

    FVFX_EffectData WeatherEffect;
    WeatherEffect.EffectType = WeatherType;
    WeatherEffect.Location = Location;
    WeatherEffect.Scale = FVector(Intensity, Intensity, Intensity);
    WeatherEffect.Duration = 0.0f; // Permanent weather
    WeatherEffect.Intensity = Intensity;

    ActiveEffects.Add(WeatherEffect);
}

void AVFX_EffectsManager::SpawnBloodSplatterVFX(FVector Location, FVector Direction)
{
    FVector Scale = FVector(1.5f, 1.5f, 0.8f);
    SpawnEffect(EVFX_EffectType::BloodSplatter, Location, Scale, 8.0f);
}

void AVFX_EffectsManager::SpawnWeaponImpactVFX(FVector Location, FVector Normal)
{
    FVector Scale = FVector(1.2f, 1.2f, 1.0f);
    SpawnEffect(EVFX_EffectType::WeaponImpact, Location, Scale, 2.0f);
}

void AVFX_EffectsManager::ClearAllEffects()
{
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("All VFX effects cleared"));
}

void AVFX_EffectsManager::SetWeatherEffectsEnabled(bool bEnabled)
{
    bEnableWeatherEffects = bEnabled;
    
    if (!bEnabled)
    {
        // Remove all weather effects
        ActiveEffects.RemoveAll([](const FVFX_EffectData& Effect)
        {
            return Effect.EffectType == EVFX_EffectType::Rain ||
                   Effect.EffectType == EVFX_EffectType::Mist ||
                   Effect.EffectType == EVFX_EffectType::Sandstorm;
        });
    }
}

void AVFX_EffectsManager::SetDinosaurVFXEnabled(bool bEnabled)
{
    bEnableDinosaurVFX = bEnabled;
    
    if (!bEnabled)
    {
        // Remove all dinosaur effects
        ActiveEffects.RemoveAll([](const FVFX_EffectData& Effect)
        {
            return Effect.EffectType == EVFX_EffectType::DinosaurFootstep ||
                   Effect.EffectType == EVFX_EffectType::DinosaurBreath;
        });
    }
}

int32 AVFX_EffectsManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void AVFX_EffectsManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove effects that have exceeded their duration (duration > 0 means temporary)
    ActiveEffects.RemoveAll([CurrentTime](const FVFX_EffectData& Effect)
    {
        return Effect.Duration > 0.0f && CurrentTime > Effect.Duration;
    });
}

void AVFX_EffectsManager::UpdateEffectLOD()
{
    // Get player location for distance calculations
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();

    // Remove effects that are too far from player
    ActiveEffects.RemoveAll([this, PlayerLocation](const FVFX_EffectData& Effect)
    {
        float Distance = FVector::Dist(PlayerLocation, Effect.Location);
        return Distance > EffectCullDistance;
    });
}

bool AVFX_EffectsManager::IsEffectInRange(const FVFX_EffectData& Effect) const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return false;

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    float Distance = FVector::Dist(PlayerLocation, Effect.Location);
    
    return Distance <= EffectCullDistance;
}