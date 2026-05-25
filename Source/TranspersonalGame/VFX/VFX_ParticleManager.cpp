#include "VFX_ParticleManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    // Initialize default values
}

void UVFX_ParticleManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Initializing prehistoric VFX system"));
    
    LoadDefaultParticleSystems();
    
    // Clear active effects arrays
    ActiveEffects.Empty();
    ActiveNiagaraEffects.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Initialization complete"));
}

void UVFX_ParticleManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Cleaning up VFX system"));
    
    // Clean up all active effects
    CleanupExpiredEffects();
    
    // Force cleanup remaining effects
    for (auto& EffectPtr : ActiveEffects)
    {
        if (EffectPtr.IsValid())
        {
            EffectPtr->DestroyComponent();
        }
    }
    
    for (auto& NiagaraPtr : ActiveNiagaraEffects)
    {
        if (NiagaraPtr.IsValid())
        {
            NiagaraPtr->DestroyComponent();
        }
    }
    
    ActiveEffects.Empty();
    ActiveNiagaraEffects.Empty();
    
    Super::Deinitialize();
}

void UVFX_ParticleManager::LoadDefaultParticleSystems()
{
    // Load default particle systems for prehistoric effects
    // Using engine defaults and legacy particle systems to avoid Niagara creation crashes
    
    // Footstep dust - use engine spark system as base
    ParticleSystemMap.Add(EVFX_ParticleType::FootstepDust, 
        TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(TEXT("/Engine/VFX/P_Sparks.P_Sparks"))));
    
    // Campfire - use engine fire system
    ParticleSystemMap.Add(EVFX_ParticleType::CampfireFire, 
        TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(TEXT("/Engine/VFX/P_Fire.P_Fire"))));
    
    // Smoke - use engine smoke system
    ParticleSystemMap.Add(EVFX_ParticleType::CampfireSmoke, 
        TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(TEXT("/Engine/VFX/P_Smoke.P_Smoke"))));
    
    // Blood splatter - use modified spark system with red tint
    ParticleSystemMap.Add(EVFX_ParticleType::BloodSplatter, 
        TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(TEXT("/Engine/VFX/P_Sparks.P_Sparks"))));
    
    // Weather rain - use engine rain if available
    ParticleSystemMap.Add(EVFX_ParticleType::WeatherRain, 
        TSoftObjectPtr<UParticleSystem>(FSoftObjectPath(TEXT("/Engine/VFX/P_Steam_Lit.P_Steam_Lit"))));
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Loaded %d default particle systems"), ParticleSystemMap.Num());
}

UParticleSystemComponent* UVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, 
    FVector Location, FRotator Rotation, const FVFX_ParticleConfig& Config)
{
    if (ParticleType == EVFX_ParticleType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Cannot spawn None particle type"));
        return nullptr;
    }
    
    UParticleSystem* ParticleSystem = GetParticleSystemForType(ParticleType);
    if (!ParticleSystem)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleManager: No particle system found for type %d"), (int32)ParticleType);
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("VFX_ParticleManager: No valid world found"));
        return nullptr;
    }
    
    // Spawn particle system component
    UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
        World, ParticleSystem, Location, Rotation, Config.Scale, Config.bAutoDestroy);
    
    if (PSC)
    {
        // Apply configuration
        if (Config.TintColor != FLinearColor::White)
        {
            // Apply color tint if supported
            PSC->SetColorParameter(FName("TintColor"), Config.TintColor);
        }
        
        // Set duration if not auto-destroy
        if (!Config.bAutoDestroy && Config.Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, 
                FTimerDelegate::CreateLambda([PSC]() {
                    if (PSC && IsValid(PSC))
                    {
                        PSC->DestroyComponent();
                    }
                }), Config.Duration, false);
        }
        
        // Track active effect
        ActiveEffects.Add(PSC);
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect type %d at %s"), 
            (int32)ParticleType, *Location.ToString());
    }
    
    return PSC;
}

void UVFX_ParticleManager::SpawnFootstepDust(FVector Location, EBiomeType BiomeType, float IntensityScale)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::FootstepDust;
    Config.Scale = FVector(IntensityScale, IntensityScale, IntensityScale) * GetBiomeParticleScale(BiomeType);
    Config.TintColor = GetBiomeParticleColor(BiomeType);
    Config.Duration = 1.5f;
    Config.bAutoDestroy = true;
    
    SpawnParticleEffect(EVFX_ParticleType::FootstepDust, Location, FRotator::ZeroRotator, Config);
}

void UVFX_ParticleManager::SpawnCampfireEffect(FVector Location)
{
    // Spawn fire effect
    FVFX_ParticleConfig FireConfig;
    FireConfig.ParticleType = EVFX_ParticleType::CampfireFire;
    FireConfig.Scale = FVector(0.8f, 0.8f, 1.2f);
    FireConfig.TintColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f); // Orange fire
    FireConfig.bAutoDestroy = false; // Persistent campfire
    
    SpawnParticleEffect(EVFX_ParticleType::CampfireFire, Location, FRotator::ZeroRotator, FireConfig);
    
    // Spawn smoke effect above fire
    FVFX_ParticleConfig SmokeConfig;
    SmokeConfig.ParticleType = EVFX_ParticleType::CampfireSmoke;
    SmokeConfig.Scale = FVector(0.6f, 0.6f, 1.0f);
    SmokeConfig.TintColor = FLinearColor(0.3f, 0.3f, 0.3f, 0.8f); // Gray smoke
    SmokeConfig.bAutoDestroy = false; // Persistent smoke
    
    FVector SmokeLocation = Location + FVector(0, 0, 50); // Offset smoke upward
    SpawnParticleEffect(EVFX_ParticleType::CampfireSmoke, SmokeLocation, FRotator::ZeroRotator, SmokeConfig);
}

void UVFX_ParticleManager::SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::BloodSplatter;
    Config.Scale = FVector(Intensity, Intensity, Intensity);
    Config.TintColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red blood
    Config.Duration = 3.0f;
    Config.bAutoDestroy = true;
    
    // Calculate rotation from direction
    FRotator BloodRotation = Direction.Rotation();
    
    SpawnParticleEffect(EVFX_ParticleType::BloodSplatter, Location, BloodRotation, Config);
}

void UVFX_ParticleManager::SpawnDinosaurBreath(FVector Location, FRotator Direction)
{
    FVFX_ParticleConfig Config;
    Config.ParticleType = EVFX_ParticleType::DinosaurBreath;
    Config.Scale = FVector(1.5f, 1.5f, 0.8f);
    Config.TintColor = FLinearColor(0.9f, 0.9f, 1.0f, 0.6f); // Misty white breath
    Config.Duration = 2.0f;
    Config.bAutoDestroy = true;
    
    SpawnParticleEffect(EVFX_ParticleType::DinosaurBreath, Location, Direction, Config);
}

void UVFX_ParticleManager::SpawnWeatherEffect(EVFX_ParticleType WeatherType, FVector Location, float Radius)
{
    if (WeatherType != EVFX_ParticleType::WeatherRain && WeatherType != EVFX_ParticleType::WeatherSnow)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Invalid weather type %d"), (int32)WeatherType);
        return;
    }
    
    FVFX_ParticleConfig Config;
    Config.ParticleType = WeatherType;
    Config.Scale = FVector(Radius / 1000.0f, Radius / 1000.0f, 1.0f);
    Config.bAutoDestroy = false; // Weather persists
    
    if (WeatherType == EVFX_ParticleType::WeatherRain)
    {
        Config.TintColor = FLinearColor(0.7f, 0.8f, 1.0f, 0.8f); // Blue-tinted rain
    }
    else if (WeatherType == EVFX_ParticleType::WeatherSnow)
    {
        Config.TintColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.9f); // White snow
    }
    
    SpawnParticleEffect(WeatherType, Location, FRotator::ZeroRotator, Config);
}

void UVFX_ParticleManager::CleanupExpiredEffects()
{
    // Remove invalid particle system components
    ActiveEffects.RemoveAll([](const TWeakObjectPtr<UParticleSystemComponent>& EffectPtr) {
        return !EffectPtr.IsValid() || !IsValid(EffectPtr.Get());
    });
    
    // Remove invalid Niagara components
    ActiveNiagaraEffects.RemoveAll([](const TWeakObjectPtr<UNiagaraComponent>& EffectPtr) {
        return !EffectPtr.IsValid() || !IsValid(EffectPtr.Get());
    });
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Active effects after cleanup: %d particle, %d Niagara"), 
        ActiveEffects.Num(), ActiveNiagaraEffects.Num());
}

UParticleSystem* UVFX_ParticleManager::GetParticleSystemForType(EVFX_ParticleType ParticleType)
{
    if (TSoftObjectPtr<UParticleSystem>* SystemPtr = ParticleSystemMap.Find(ParticleType))
    {
        if (SystemPtr->IsValid())
        {
            return SystemPtr->Get();
        }
        else if (!SystemPtr->IsNull())
        {
            // Try to load the asset
            return SystemPtr->LoadSynchronous();
        }
    }
    
    return nullptr;
}

FLinearColor UVFX_ParticleManager::GetBiomeParticleColor(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return FLinearColor(0.9f, 0.7f, 0.4f, 1.0f); // Sandy dust
        case EBiomeType::Floresta:
            return FLinearColor(0.4f, 0.3f, 0.2f, 1.0f); // Dark soil
        case EBiomeType::Pantano:
            return FLinearColor(0.3f, 0.4f, 0.2f, 1.0f); // Muddy green
        case EBiomeType::Deserto:
            return FLinearColor(1.0f, 0.8f, 0.5f, 1.0f); // Light sand
        case EBiomeType::Montanha:
            return FLinearColor(0.6f, 0.6f, 0.7f, 1.0f); // Rocky gray
        default:
            return FLinearColor::White;
    }
}

float UVFX_ParticleManager::GetBiomeParticleScale(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            return 1.2f; // More dust in dry savanna
        case EBiomeType::Floresta:
            return 0.8f; // Less dust in forest
        case EBiomeType::Pantano:
            return 1.5f; // More splash in swamp
        case EBiomeType::Deserto:
            return 1.4f; // More sand particles
        case EBiomeType::Montanha:
            return 0.9f; // Rocky terrain, less dust
        default:
            return 1.0f;
    }
}