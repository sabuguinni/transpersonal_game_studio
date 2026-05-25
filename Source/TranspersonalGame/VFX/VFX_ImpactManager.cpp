#include "VFX_ImpactManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    DefaultEffectDuration = 3.0f;
    bAutoCleanupEffects = true;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeDefaultEffects();
}

void UVFX_ImpactManager::InitializeDefaultEffects()
{
    // Initialize default impact data
    FVFX_ImpactData BloodData;
    BloodData.Scale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 4.0f;
    ImpactEffects.Add(EVFX_ImpactType::Blood, BloodData);

    FVFX_ImpactData DustData;
    DustData.Scale = FVector(2.0f, 2.0f, 1.5f);
    DustData.Duration = 3.0f;
    ImpactEffects.Add(EVFX_ImpactType::Dust, DustData);

    FVFX_ImpactData RockData;
    RockData.Scale = FVector(1.5f, 1.5f, 1.0f);
    RockData.Duration = 2.5f;
    ImpactEffects.Add(EVFX_ImpactType::Rock, RockData);

    FVFX_ImpactData WaterData;
    WaterData.Scale = FVector(1.2f, 1.2f, 0.8f);
    WaterData.Duration = 2.0f;
    ImpactEffects.Add(EVFX_ImpactType::Water, WaterData);

    FVFX_ImpactData MudData;
    MudData.Scale = FVector(1.8f, 1.8f, 1.2f);
    MudData.Duration = 3.5f;
    ImpactEffects.Add(EVFX_ImpactType::Mud, MudData);

    UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Default effects initialized"));
}

void UVFX_ImpactManager::SpawnImpactEffect(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation)
{
    if (!ImpactEffects.Contains(ImpactType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: Impact type not found"));
        return;
    }

    const FVFX_ImpactData& EffectData = ImpactEffects[ImpactType];
    
    if (EffectData.ParticleEffect)
    {
        UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            EffectData.ParticleEffect,
            Location,
            Rotation,
            EffectData.Scale,
            true
        );

        if (ParticleComp && bAutoCleanupEffects)
        {
            ActiveEffects.Add(ParticleComp);
            
            // Schedule cleanup
            FTimerHandle CleanupTimer;
            GetWorld()->GetTimerManager().SetTimer(CleanupTimer, [this, ParticleComp]()
            {
                if (IsValid(ParticleComp))
                {
                    ParticleComp->DestroyComponent();
                    ActiveEffects.Remove(ParticleComp);
                }
            }, EffectData.Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX_ImpactManager: Spawned impact effect at %s"), *Location.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_ImpactManager: No particle effect assigned for impact type"));
    }
}

void UVFX_ImpactManager::SpawnBloodSplatter(FVector Location, FVector ImpactDirection, float Intensity)
{
    // Calculate rotation based on impact direction
    FRotator EffectRotation = ImpactDirection.Rotation();
    
    // Adjust scale based on intensity
    if (ImpactEffects.Contains(EVFX_ImpactType::Blood))
    {
        FVFX_ImpactData BloodData = ImpactEffects[EVFX_ImpactType::Blood];
        BloodData.Scale *= Intensity;
        
        // Temporarily update the effect data
        ImpactEffects[EVFX_ImpactType::Blood] = BloodData;
        SpawnImpactEffect(EVFX_ImpactType::Blood, Location, EffectRotation);
        
        // Restore original scale
        BloodData.Scale /= Intensity;
        ImpactEffects[EVFX_ImpactType::Blood] = BloodData;
    }
}

void UVFX_ImpactManager::SpawnDustCloud(FVector Location, float Size)
{
    if (ImpactEffects.Contains(EVFX_ImpactType::Dust))
    {
        FVFX_ImpactData DustData = ImpactEffects[EVFX_ImpactType::Dust];
        DustData.Scale *= Size;
        
        // Temporarily update the effect data
        ImpactEffects[EVFX_ImpactType::Dust] = DustData;
        SpawnImpactEffect(EVFX_ImpactType::Dust, Location);
        
        // Restore original scale
        DustData.Scale /= Size;
        ImpactEffects[EVFX_ImpactType::Dust] = DustData;
    }
}

void UVFX_ImpactManager::CleanupExpiredEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; i--)
    {
        if (!IsValid(ActiveEffects[i]))
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}