#include "VFXSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

UVFX_SystemManager::UVFX_SystemManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetComponentTickEnabled(false);
}

void UVFX_SystemManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectDatabase();
}

void UVFX_SystemManager::InitializeEffectDatabase()
{
    EffectDatabase.Empty();

    // Fire effects
    FVFX_EffectData CampfireData;
    CampfireData.EffectType = EVFX_EffectType::Fire_Campfire;
    CampfireData.Scale = FVector(1.0f, 1.0f, 1.5f);
    CampfireData.Duration = -1.0f; // Infinite
    CampfireData.bAutoDestroy = false;
    EffectDatabase.Add(CampfireData);

    // Dust effects
    FVFX_EffectData FootstepData;
    FootstepData.EffectType = EVFX_EffectType::Dust_Footstep;
    FootstepData.Scale = FVector(1.0f);
    FootstepData.Duration = 2.0f;
    FootstepData.bAutoDestroy = true;
    EffectDatabase.Add(FootstepData);

    // Water effects
    FVFX_EffectData WaterData;
    WaterData.EffectType = EVFX_EffectType::Water_Splash;
    WaterData.Scale = FVector(1.0f);
    WaterData.Duration = 3.0f;
    WaterData.bAutoDestroy = true;
    EffectDatabase.Add(WaterData);

    // Breath vapor
    FVFX_EffectData BreathData;
    BreathData.EffectType = EVFX_EffectType::Breath_Vapor;
    BreathData.Scale = FVector(0.8f, 0.8f, 1.2f);
    BreathData.Duration = 4.0f;
    BreathData.bAutoDestroy = true;
    EffectDatabase.Add(BreathData);

    // Volcanic ash
    FVFX_EffectData AshData;
    AshData.EffectType = EVFX_EffectType::Volcanic_Ash;
    AshData.Scale = FVector(2.0f, 2.0f, 3.0f);
    AshData.Duration = -1.0f; // Environmental effect
    AshData.bAutoDestroy = false;
    EffectDatabase.Add(AshData);

    UE_LOG(LogTemp, Warning, TEXT("VFX System Manager: Initialized %d effect types"), EffectDatabase.Num());
}

UNiagaraComponent* UVFX_SystemManager::SpawnVFXEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    FVFX_EffectData* EffectData = FindEffectData(EffectType);
    if (!EffectData)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System Manager: Effect type not found"));
        return nullptr;
    }

    UNiagaraSystem* NiagaraSystem = EffectData->NiagaraSystem.LoadSynchronous();
    if (!NiagaraSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX System Manager: Niagara system not loaded"));
        return nullptr;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Location,
        Rotation,
        Scale * EffectData->Scale,
        EffectData->bAutoDestroy
    );

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        
        if (EffectData->Duration > 0.0f && EffectData->bAutoDestroy)
        {
            // Auto-destroy after duration
            FTimerHandle TimerHandle;
            World->GetTimerManager().SetTimer(TimerHandle, [this, NiagaraComp]()
            {
                if (IsValid(NiagaraComp))
                {
                    NiagaraComp->DestroyComponent();
                    ActiveEffects.Remove(NiagaraComp);
                }
            }, EffectData->Duration, false);
        }

        UE_LOG(LogTemp, Log, TEXT("VFX System Manager: Spawned effect at location %s"), *Location.ToString());
    }

    return NiagaraComp;
}

void UVFX_SystemManager::StopVFXEffect(UNiagaraComponent* Effect)
{
    if (IsValid(Effect))
    {
        Effect->Deactivate();
        ActiveEffects.Remove(Effect);
    }
}

void UVFX_SystemManager::StopAllEffects()
{
    for (UNiagaraComponent* Effect : ActiveEffects)
    {
        if (IsValid(Effect))
        {
            Effect->Deactivate();
        }
    }
    ActiveEffects.Empty();
}

UNiagaraComponent* UVFX_SystemManager::SpawnDinosaurBreathEffect(FVector Location, FRotator Direction)
{
    FVector BreathLocation = Location + Direction.Vector() * 200.0f; // In front of dinosaur mouth
    return SpawnVFXEffect(EVFX_EffectType::Breath_Vapor, BreathLocation, Direction, FVector(1.5f));
}

UNiagaraComponent* UVFX_SystemManager::SpawnFootstepDustEffect(FVector Location, float DinosaurSize)
{
    FVector DustScale = FVector(DinosaurSize, DinosaurSize, 1.0f);
    return SpawnVFXEffect(EVFX_EffectType::Dust_Footstep, Location, FRotator::ZeroRotator, DustScale);
}

UNiagaraComponent* UVFX_SystemManager::SpawnCampfireEffect(FVector Location)
{
    return SpawnVFXEffect(EVFX_EffectType::Fire_Campfire, Location, FRotator::ZeroRotator, FVector(1.0f));
}

FVFX_EffectData* UVFX_SystemManager::FindEffectData(EVFX_EffectType EffectType)
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

void UVFX_SystemManager::CleanupDestroyedEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Effect)
    {
        return !IsValid(Effect);
    });
}