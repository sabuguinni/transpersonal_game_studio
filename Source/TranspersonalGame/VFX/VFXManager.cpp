#include "VFXManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = false;
    InitializeEffectDatabase();
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    
    // Start cleanup timer for expired effects
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &UVFX_Manager::CleanupExpiredEffects,
            5.0f,  // Check every 5 seconds
            true   // Loop
        );
    }
}

void UVFX_Manager::InitializeEffectDatabase()
{
    EffectDatabase.Empty();

    // Blood splatter effect
    FVFX_EffectData BloodData;
    BloodData.EffectType = EVFX_EffectType::BloodSplatter;
    BloodData.DefaultScale = FVector(1.0f, 1.0f, 1.0f);
    BloodData.Duration = 3.0f;
    EffectDatabase.Add(BloodData);

    // Footstep dust effect
    FVFX_EffectData DustData;
    DustData.EffectType = EVFX_EffectType::FootstepDust;
    DustData.DefaultScale = FVector(2.0f, 2.0f, 1.0f);
    DustData.Duration = 2.0f;
    EffectDatabase.Add(DustData);

    // Volcanic ash effect
    FVFX_EffectData AshData;
    AshData.EffectType = EVFX_EffectType::VolcanicAsh;
    AshData.DefaultScale = FVector(10.0f, 10.0f, 5.0f);
    AshData.Duration = 30.0f;
    EffectDatabase.Add(AshData);

    // Campfire smoke effect
    FVFX_EffectData SmokeData;
    SmokeData.EffectType = EVFX_EffectType::CampfireSmoke;
    SmokeData.DefaultScale = FVector(1.5f, 1.5f, 2.0f);
    SmokeData.Duration = -1.0f; // Continuous
    EffectDatabase.Add(SmokeData);

    // Rain drops effect
    FVFX_EffectData RainData;
    RainData.EffectType = EVFX_EffectType::RainDrops;
    RainData.DefaultScale = FVector(20.0f, 20.0f, 10.0f);
    RainData.Duration = -1.0f; // Continuous
    EffectDatabase.Add(RainData);

    // Dinosaur breath effect
    FVFX_EffectData BreathData;
    BreathData.EffectType = EVFX_EffectType::DinosaurBreath;
    BreathData.DefaultScale = FVector(1.0f, 1.0f, 1.0f);
    BreathData.Duration = 1.5f;
    EffectDatabase.Add(BreathData);
}

void UVFX_Manager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    FVFX_EffectData* EffectData = GetEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFXManager: Effect type not found in database"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Try Niagara system first
    if (EffectData->NiagaraSystem.IsValid())
    {
        UNiagaraSystem* NiagaraAsset = EffectData->NiagaraSystem.LoadSynchronous();
        if (NiagaraAsset)
        {
            UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                World,
                NiagaraAsset,
                Location,
                Rotation,
                Scale * EffectData->DefaultScale
            );

            if (NiagaraComp)
            {
                ActiveNiagaraEffects.Add(NiagaraComp);
                UE_LOG(LogTemp, Log, TEXT("VFXManager: Spawned Niagara effect at %s"), *Location.ToString());
            }
        }
    }
    // Fallback to particle system
    else if (EffectData->ParticleSystem.IsValid())
    {
        UParticleSystem* ParticleAsset = EffectData->ParticleSystem.LoadSynchronous();
        if (ParticleAsset)
        {
            UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
                World,
                ParticleAsset,
                Location,
                Rotation,
                Scale * EffectData->DefaultScale
            );

            if (ParticleComp)
            {
                ActiveParticleEffects.Add(ParticleComp);
                UE_LOG(LogTemp, Log, TEXT("VFXManager: Spawned particle effect at %s"), *Location.ToString());
            }
        }
    }
}

void UVFX_Manager::SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactNormal)
{
    // Calculate rotation from impact normal
    FRotator ImpactRotation = FRotationMatrix::MakeFromZ(ImpactNormal).Rotator();
    
    SpawnEffect(EVFX_EffectType::BloodSplatter, ImpactLocation, ImpactRotation);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Blood splatter at %s"), *ImpactLocation.ToString());
}

void UVFX_Manager::SpawnFootstepDust(FVector FootLocation, float DinosaurSize)
{
    FVector DustScale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    SpawnEffect(EVFX_EffectType::FootstepDust, FootLocation, FRotator::ZeroRotator, DustScale);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Footstep dust at %s (size: %f)"), *FootLocation.ToString(), DinosaurSize);
}

void UVFX_Manager::SpawnVolcanicAsh(FVector VolcanoLocation, float Intensity)
{
    FVector AshScale = FVector(Intensity, Intensity, Intensity * 0.5f);
    SpawnEffect(EVFX_EffectType::VolcanicAsh, VolcanoLocation, FRotator::ZeroRotator, AshScale);
    
    UE_LOG(LogTemp, Log, TEXT("VFXManager: Volcanic ash at %s (intensity: %f)"), *VolcanoLocation.ToString(), Intensity);
}

void UVFX_Manager::CleanupExpiredEffects()
{
    // Clean up finished Niagara effects
    ActiveNiagaraEffects.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });

    // Clean up finished particle effects
    ActiveParticleEffects.RemoveAll([](UParticleSystemComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });

    UE_LOG(LogTemp, Verbose, TEXT("VFXManager: Active effects - Niagara: %d, Particles: %d"), 
           ActiveNiagaraEffects.Num(), ActiveParticleEffects.Num());
}

FVFX_EffectData* UVFX_Manager::GetEffectData(EVFX_EffectType EffectType)
{
    for (FVFX_EffectData& Data : EffectDatabase)
    {
        if (Data.EffectType == EffectType)
        {
            return &Data;
        }
    }
    return nullptr;
}