#include "VFX_ParticleManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UVFX_ParticleManager::UVFX_ParticleManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    bEnableVFX = true;
    GlobalVFXScale = 1.0f;
    LastCleanupTime = 0.0f;
    
    InitializeDefaultConfigs();
}

void UVFX_ParticleManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: BeginPlay - Prehistoric VFX system initialized"));
    
    // Initialize particle configs if empty
    if (ParticleConfigs.Num() == 0)
    {
        InitializeDefaultConfigs();
    }
}

void UVFX_ParticleManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableVFX)
    {
        return;
    }
    
    // Periodic cleanup of expired particles
    LastCleanupTime += DeltaTime;
    if (LastCleanupTime >= CleanupInterval)
    {
        CleanupExpiredParticles();
        LastCleanupTime = 0.0f;
    }
}

void UVFX_ParticleManager::InitializeDefaultConfigs()
{
    ParticleConfigs.Empty();
    
    // Campfire configuration
    FVFX_ParticleConfig CampfireConfig;
    CampfireConfig.ParticleType = EVFX_ParticleType::Fire_Campfire;
    CampfireConfig.Scale = FVector(1.5f, 1.5f, 2.0f);
    CampfireConfig.Duration = 0.0f; // Continuous
    CampfireConfig.bAutoDestroy = false;
    CampfireConfig.SpawnRate = 75.0f;
    ParticleConfigs.Add(CampfireConfig);
    
    // Footstep dust configuration
    FVFX_ParticleConfig FootstepConfig;
    FootstepConfig.ParticleType = EVFX_ParticleType::Dust_Footstep;
    FootstepConfig.Scale = FVector(1.0f, 1.0f, 0.5f);
    FootstepConfig.Duration = 2.0f;
    FootstepConfig.bAutoDestroy = true;
    FootstepConfig.SpawnRate = 100.0f;
    ParticleConfigs.Add(FootstepConfig);
    
    // Blood splatter configuration
    FVFX_ParticleConfig BloodConfig;
    BloodConfig.ParticleType = EVFX_ParticleType::Blood_Impact;
    BloodConfig.Scale = FVector(0.8f, 0.8f, 0.8f);
    BloodConfig.Duration = 1.5f;
    BloodConfig.bAutoDestroy = true;
    BloodConfig.SpawnRate = 150.0f;
    ParticleConfigs.Add(BloodConfig);
    
    // Water splash configuration
    FVFX_ParticleConfig WaterConfig;
    WaterConfig.ParticleType = EVFX_ParticleType::Water_Splash;
    WaterConfig.Scale = FVector(1.2f, 1.2f, 1.0f);
    WaterConfig.Duration = 3.0f;
    WaterConfig.bAutoDestroy = true;
    WaterConfig.SpawnRate = 80.0f;
    ParticleConfigs.Add(WaterConfig);
    
    // Forest mist configuration
    FVFX_ParticleConfig MistConfig;
    MistConfig.ParticleType = EVFX_ParticleType::Mist_Forest;
    MistConfig.Scale = FVector(3.0f, 3.0f, 1.5f);
    MistConfig.Duration = 0.0f; // Continuous
    MistConfig.bAutoDestroy = false;
    MistConfig.SpawnRate = 25.0f;
    ParticleConfigs.Add(MistConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Initialized %d default particle configs"), ParticleConfigs.Num());
}

FVFX_ParticleConfig* UVFX_ParticleManager::GetParticleConfig(EVFX_ParticleType ParticleType)
{
    for (FVFX_ParticleConfig& Config : ParticleConfigs)
    {
        if (Config.ParticleType == ParticleType)
        {
            return &Config;
        }
    }
    return nullptr;
}

UNiagaraComponent* UVFX_ParticleManager::SpawnParticleEffect(EVFX_ParticleType ParticleType, FVector Location, FRotator Rotation)
{
    if (!bEnableVFX)
    {
        return nullptr;
    }
    
    FVFX_ParticleConfig* Config = GetParticleConfig(ParticleType);
    if (!Config)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: No config found for particle type"));
        return nullptr;
    }
    
    // For now, create a basic Niagara component without loading external assets
    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(),
        nullptr, // Will be set when Niagara systems are created
        Location,
        Rotation,
        Config->Scale * GlobalVFXScale,
        true,
        Config->bAutoDestroy
    );
    
    if (NiagaraComp)
    {
        ActiveParticles.Add(NiagaraComp);
        
        // Set duration for auto-destroying particles
        if (Config->bAutoDestroy && Config->Duration > 0.0f)
        {
            FTimerHandle TimerHandle;
            GetWorld()->GetTimerManager().SetTimer(TimerHandle, [NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                }
            }, Config->Duration, false);
        }
        
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned particle effect at location %s"), *Location.ToString());
    }
    
    return NiagaraComp;
}

void UVFX_ParticleManager::SpawnFootstepDust(FVector FootLocation, float DinosaurSize)
{
    if (!bEnableVFX)
    {
        return;
    }
    
    // Adjust dust scale based on dinosaur size
    FVector DustLocation = FootLocation + FVector(0.0f, 0.0f, -10.0f); // Slightly below foot
    
    FVFX_ParticleConfig* Config = GetParticleConfig(EVFX_ParticleType::Dust_Footstep);
    if (Config)
    {
        FVector OriginalScale = Config->Scale;
        Config->Scale *= DinosaurSize; // Scale dust based on dinosaur size
        
        SpawnParticleEffect(EVFX_ParticleType::Dust_Footstep, DustLocation);
        
        Config->Scale = OriginalScale; // Restore original scale
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned footstep dust for dinosaur size %.2f"), DinosaurSize);
}

void UVFX_ParticleManager::SpawnCampfire(FVector FireLocation)
{
    if (!bEnableVFX)
    {
        return;
    }
    
    SpawnParticleEffect(EVFX_ParticleType::Fire_Campfire, FireLocation);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned campfire at location %s"), *FireLocation.ToString());
}

void UVFX_ParticleManager::SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactDirection)
{
    if (!bEnableVFX)
    {
        return;
    }
    
    // Calculate rotation based on impact direction
    FRotator BloodRotation = ImpactDirection.Rotation();
    
    SpawnParticleEffect(EVFX_ParticleType::Blood_Impact, ImpactLocation, BloodRotation);
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned blood splatter at impact location %s"), *ImpactLocation.ToString());
}

void UVFX_ParticleManager::SpawnWaterSplash(FVector WaterLocation, float SplashIntensity)
{
    if (!bEnableVFX)
    {
        return;
    }
    
    FVFX_ParticleConfig* Config = GetParticleConfig(EVFX_ParticleType::Water_Splash);
    if (Config)
    {
        FVector OriginalScale = Config->Scale;
        Config->Scale *= SplashIntensity; // Scale splash based on intensity
        
        SpawnParticleEffect(EVFX_ParticleType::Water_Splash, WaterLocation);
        
        Config->Scale = OriginalScale; // Restore original scale
    }
    
    UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Spawned water splash with intensity %.2f"), SplashIntensity);
}

void UVFX_ParticleManager::StopAllParticles()
{
    for (UNiagaraComponent* Particle : ActiveParticles)
    {
        if (IsValid(Particle))
        {
            Particle->DeactivateImmediate();
            Particle->DestroyComponent();
        }
    }
    
    ActiveParticles.Empty();
    UE_LOG(LogTemp, Warning, TEXT("VFX_ParticleManager: Stopped all active particles"));
}

void UVFX_ParticleManager::CleanupExpiredParticles()
{
    int32 RemovedCount = 0;
    
    for (int32 i = ActiveParticles.Num() - 1; i >= 0; i--)
    {
        UNiagaraComponent* Particle = ActiveParticles[i];
        if (!IsValid(Particle) || !Particle->IsActive())
        {
            ActiveParticles.RemoveAt(i);
            RemovedCount++;
        }
    }
    
    if (RemovedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("VFX_ParticleManager: Cleaned up %d expired particles. Active: %d"), RemovedCount, ActiveParticles.Num());
    }
}