// NiagaraVFXManager.cpp
// VFX Agent #17 — Transpersonal Game Studio
// Implements the Niagara VFX Manager: spawning, LOD, pooling, and lifecycle for all prehistoric VFX.

#include "NiagaraVFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UVFX_NiagaraVFXManager — Implementation
// ============================================================

UVFX_NiagaraVFXManager::UVFX_NiagaraVFXManager()
{
    // Default LOD distance thresholds
    LODHighMaxDistance    = 1500.0f;
    LODMediumMaxDistance  = 4000.0f;
    LODLowMaxDistance     = 8000.0f;
    MaxActiveEffects      = 64;
    bVFXEnabled           = true;
}

void UVFX_NiagaraVFXManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveEffects.Reserve(MaxActiveEffects);
    UE_LOG(LogTemp, Log, TEXT("[VFX] NiagaraVFXManager initialized. MaxActiveEffects=%d"), MaxActiveEffects);
}

void UVFX_NiagaraVFXManager::Deinitialize()
{
    // Clean up all active Niagara components
    for (FVFX_ActiveEffect& Entry : ActiveEffects)
    {
        if (Entry.NiagaraComponent && Entry.NiagaraComponent->IsValidLowLevel())
        {
            Entry.NiagaraComponent->DeactivateImmediate();
        }
    }
    ActiveEffects.Empty();
    Super::Deinitialize();
}

UNiagaraComponent* UVFX_NiagaraVFXManager::SpawnEffect(
    EVFX_EffectType EffectType,
    const FVector& Location,
    const FRotator& Rotation,
    AActor* AttachTarget)
{
    if (!bVFXEnabled)
    {
        return nullptr;
    }

    UNiagaraSystem* System = GetSystemForEffect(EffectType);
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFX] No Niagara system registered for effect type %d"), (int32)EffectType);
        return nullptr;
    }

    // Enforce pool limit — cull oldest if at capacity
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CullOldestEffect();
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = nullptr;

    if (AttachTarget)
    {
        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            System,
            AttachTarget->GetRootComponent(),
            NAME_None,
            Location,
            Rotation,
            EAttachLocation::KeepWorldPosition,
            true
        );
    }
    else
    {
        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            System,
            Location,
            Rotation
        );
    }

    if (NiagaraComp)
    {
        FVFX_ActiveEffect NewEntry;
        NewEntry.NiagaraComponent = NiagaraComp;
        NewEntry.EffectType        = EffectType;
        NewEntry.SpawnLocation     = Location;
        NewEntry.SpawnTime         = World->GetTimeSeconds();
        NewEntry.CurrentLOD        = EVFX_LODTier::High;
        ActiveEffects.Add(NewEntry);

        UE_LOG(LogTemp, Verbose, TEXT("[VFX] Spawned effect type=%d at %s"), (int32)EffectType, *Location.ToString());
    }

    return NiagaraComp;
}

void UVFX_NiagaraVFXManager::StopEffect(UNiagaraComponent* NiagaraComponent, bool bImmediate)
{
    if (!NiagaraComponent || !NiagaraComponent->IsValidLowLevel())
    {
        return;
    }

    if (bImmediate)
    {
        NiagaraComponent->DeactivateImmediate();
    }
    else
    {
        NiagaraComponent->Deactivate();
    }

    // Remove from active list
    ActiveEffects.RemoveAll([NiagaraComponent](const FVFX_ActiveEffect& Entry)
    {
        return Entry.NiagaraComponent == NiagaraComponent;
    });
}

void UVFX_NiagaraVFXManager::RegisterEffectSystem(EVFX_EffectType EffectType, UNiagaraSystem* System)
{
    if (!System)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFX] Attempted to register null NiagaraSystem for effect type %d"), (int32)EffectType);
        return;
    }
    EffectSystemMap.Add(EffectType, System);
    UE_LOG(LogTemp, Log, TEXT("[VFX] Registered Niagara system '%s' for effect type %d"), *System->GetName(), (int32)EffectType);
}

void UVFX_NiagaraVFXManager::UpdateLOD(const FVector& CameraLocation)
{
    for (FVFX_ActiveEffect& Entry : ActiveEffects)
    {
        if (!Entry.NiagaraComponent || !Entry.NiagaraComponent->IsValidLowLevel())
        {
            continue;
        }

        float Distance = FVector::Dist(CameraLocation, Entry.SpawnLocation);
        EVFX_LODTier NewLOD = ComputeLODTier(Distance);

        if (NewLOD != Entry.CurrentLOD)
        {
            Entry.CurrentLOD = NewLOD;
            ApplyLODToComponent(Entry.NiagaraComponent, NewLOD);
        }
    }
}

EVFX_LODTier UVFX_NiagaraVFXManager::ComputeLODTier(float Distance) const
{
    if (Distance <= LODHighMaxDistance)
    {
        return EVFX_LODTier::High;
    }
    else if (Distance <= LODMediumMaxDistance)
    {
        return EVFX_LODTier::Medium;
    }
    else if (Distance <= LODLowMaxDistance)
    {
        return EVFX_LODTier::Low;
    }
    return EVFX_LODTier::Culled;
}

void UVFX_NiagaraVFXManager::ApplyLODToComponent(UNiagaraComponent* Component, EVFX_LODTier LODTier)
{
    if (!Component)
    {
        return;
    }

    switch (LODTier)
    {
        case EVFX_LODTier::High:
            Component->SetVisibility(true);
            Component->SetPaused(false);
            break;

        case EVFX_LODTier::Medium:
            Component->SetVisibility(true);
            Component->SetPaused(false);
            // Medium LOD: reduce simulation quality via Niagara scalability
            Component->SetVariableFloat(FName("SpawnRateScale"), 0.5f);
            break;

        case EVFX_LODTier::Low:
            Component->SetVisibility(true);
            Component->SetPaused(false);
            Component->SetVariableFloat(FName("SpawnRateScale"), 0.2f);
            break;

        case EVFX_LODTier::Culled:
            Component->SetVisibility(false);
            Component->SetPaused(true);
            break;
    }
}

UNiagaraSystem* UVFX_NiagaraVFXManager::GetSystemForEffect(EVFX_EffectType EffectType) const
{
    const TObjectPtr<UNiagaraSystem>* Found = EffectSystemMap.Find(EffectType);
    return Found ? Found->Get() : nullptr;
}

void UVFX_NiagaraVFXManager::CullOldestEffect()
{
    if (ActiveEffects.Num() == 0)
    {
        return;
    }

    // Find oldest by spawn time
    int32 OldestIndex = 0;
    float OldestTime = ActiveEffects[0].SpawnTime;

    for (int32 i = 1; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].SpawnTime < OldestTime)
        {
            OldestTime = ActiveEffects[i].SpawnTime;
            OldestIndex = i;
        }
    }

    FVFX_ActiveEffect& Oldest = ActiveEffects[OldestIndex];
    if (Oldest.NiagaraComponent && Oldest.NiagaraComponent->IsValidLowLevel())
    {
        Oldest.NiagaraComponent->DeactivateImmediate();
    }
    ActiveEffects.RemoveAt(OldestIndex);
    UE_LOG(LogTemp, Verbose, TEXT("[VFX] Culled oldest effect to maintain pool limit"));
}

int32 UVFX_NiagaraVFXManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

void UVFX_NiagaraVFXManager::StopAllEffects(bool bImmediate)
{
    for (FVFX_ActiveEffect& Entry : ActiveEffects)
    {
        if (Entry.NiagaraComponent && Entry.NiagaraComponent->IsValidLowLevel())
        {
            if (bImmediate)
            {
                Entry.NiagaraComponent->DeactivateImmediate();
            }
            else
            {
                Entry.NiagaraComponent->Deactivate();
            }
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("[VFX] All effects stopped."));
}
