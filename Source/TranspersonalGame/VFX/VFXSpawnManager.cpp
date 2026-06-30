#include "VFXSpawnManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "TimerManager.h"

// ============================================================
// UVFX_SpawnManager — Implementation
// VFX Agent #17 | Cycle AUTO_20260630_011
// ============================================================

void UVFX_SpawnManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ActiveEffects.Reserve(MaxConcurrentEffects);
    UE_LOG(LogTemp, Log, TEXT("[VFX_SpawnManager] Initialized. MaxConcurrentEffects=%d"), MaxConcurrentEffects);
}

void UVFX_SpawnManager::Deinitialize()
{
    StopAllEffectsImmediate();
    ActiveEffects.Empty();
    Super::Deinitialize();
    UE_LOG(LogTemp, Log, TEXT("[VFX_SpawnManager] Deinitialized."));
}

bool UVFX_SpawnManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

// --- Core Spawn API ---

int32 UVFX_SpawnManager::SpawnEffect(const FVFX_SpawnRequest& Request)
{
    if (ActiveEffects.Num() >= MaxConcurrentEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFX_SpawnManager] Effect pool full (%d/%d). Skipping spawn."),
            ActiveEffects.Num(), MaxConcurrentEffects);
        return -1;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return -1;
    }

    int32 NewID = GenerateEffectID();

    FVFX_ActiveEffect NewEffect;
    NewEffect.EffectID = NewID;
    NewEffect.Category = Request.Category;
    NewEffect.Location = Request.Location;
    NewEffect.SpawnTime = World->GetTimeSeconds();
    NewEffect.LifetimeSeconds = Request.LifetimeSeconds;
    NewEffect.CurrentLOD = GetLODForDistance(GetDistanceToCamera(Request.Location));
    NewEffect.EffectActor = nullptr; // Niagara actor would be set here when Niagara module is available

    ActiveEffects.Add(NewID, NewEffect);

    UE_LOG(LogTemp, Log, TEXT("[VFX_SpawnManager] Spawned effect ID=%d Category=%d LOD=%d at (%.0f,%.0f,%.0f)"),
        NewID,
        (int32)Request.Category,
        (int32)NewEffect.CurrentLOD,
        Request.Location.X, Request.Location.Y, Request.Location.Z);

    // Schedule auto-destroy
    if (Request.bAutoDestroy && Request.LifetimeSeconds > 0.0f)
    {
        FTimerHandle TimerHandle;
        FTimerDelegate TimerDel;
        TimerDel.BindUObject(this, &UVFX_SpawnManager::StopEffect, NewID, false);
        World->GetTimerManager().SetTimer(TimerHandle, TimerDel, Request.LifetimeSeconds, false);
    }

    return NewID;
}

void UVFX_SpawnManager::StopEffect(int32 EffectID, bool bImmediate)
{
    FVFX_ActiveEffect* Effect = ActiveEffects.Find(EffectID);
    if (!Effect)
    {
        return;
    }

    if (Effect->EffectActor && !Effect->EffectActor->IsActorBeingDestroyed())
    {
        if (bImmediate)
        {
            Effect->EffectActor->Destroy();
        }
        else
        {
            // Graceful deactivation — let the effect finish its current loop
            Effect->EffectActor->SetActorHiddenInGame(true);
            Effect->EffectActor->SetActorEnableCollision(false);
        }
    }

    ActiveEffects.Remove(EffectID);
    UE_LOG(LogTemp, Verbose, TEXT("[VFX_SpawnManager] Stopped effect ID=%d (immediate=%d)"), EffectID, (int32)bImmediate);
}

void UVFX_SpawnManager::StopAllEffects(EVFX_SpawnCategory Category)
{
    TArray<int32> ToRemove;
    for (auto& Pair : ActiveEffects)
    {
        if (Pair.Value.Category == Category)
        {
            ToRemove.Add(Pair.Key);
        }
    }
    for (int32 ID : ToRemove)
    {
        StopEffect(ID, false);
    }
    UE_LOG(LogTemp, Log, TEXT("[VFX_SpawnManager] Stopped %d effects in category %d"), ToRemove.Num(), (int32)Category);
}

void UVFX_SpawnManager::StopAllEffectsImmediate()
{
    TArray<int32> AllIDs;
    ActiveEffects.GetKeys(AllIDs);
    for (int32 ID : AllIDs)
    {
        StopEffect(ID, true);
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("[VFX_SpawnManager] All effects stopped immediately."));
}

// --- LOD Management ---

EVFX_LODLevel UVFX_SpawnManager::GetLODForDistance(float DistanceFromCamera) const
{
    if (!bEnableLODScaling)
    {
        return EVFX_LODLevel::High;
    }

    if (DistanceFromCamera < LODDistanceHigh)
    {
        return EVFX_LODLevel::High;
    }
    else if (DistanceFromCamera < LODDistanceMedium)
    {
        return EVFX_LODLevel::Medium;
    }
    else if (DistanceFromCamera < LODDistanceLow)
    {
        return EVFX_LODLevel::Low;
    }
    return EVFX_LODLevel::Culled;
}

void UVFX_SpawnManager::UpdateAllEffectLODs()
{
    for (auto& Pair : ActiveEffects)
    {
        FVFX_ActiveEffect& Effect = Pair.Value;
        float Dist = GetDistanceToCamera(Effect.Location);
        EVFX_LODLevel NewLOD = GetLODForDistance(Dist);
        if (NewLOD != Effect.CurrentLOD)
        {
            ApplyLODToEffect(Effect, NewLOD);
            Effect.CurrentLOD = NewLOD;
        }
    }
}

// --- Convenience Spawners ---

int32 UVFX_SpawnManager::SpawnCampfireEffect(FVector Location, float Scale)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Environment;
    Req.Location = Location;
    Req.Scale = Scale;
    Req.LifetimeSeconds = 0.0f; // Persistent
    Req.bAutoDestroy = false;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnFootstepDust(FVector ImpactLocation, float DinoMassKg)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Dinosaur;
    Req.Location = ImpactLocation;
    // Scale dust cloud by dino mass — heavier dino = bigger dust
    Req.Scale = FMath::Clamp(DinoMassKg / 500.0f, 0.5f, 4.0f);
    Req.LifetimeSeconds = 1.5f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnBloodSplatter(FVector ImpactLocation, FVector ImpactNormal, float DamageAmount)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Combat;
    Req.Location = ImpactLocation;
    Req.Rotation = ImpactNormal.Rotation();
    Req.Scale = FMath::Clamp(DamageAmount / 25.0f, 0.3f, 3.0f);
    Req.LifetimeSeconds = 2.0f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnRainSplash(FVector Location)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Weather;
    Req.Location = Location;
    Req.Scale = 1.0f;
    Req.LifetimeSeconds = 0.4f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnVolcanicAsh(FVector Location, float Intensity)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Volcanic;
    Req.Location = Location;
    Req.Scale = FMath::Clamp(Intensity, 0.1f, 5.0f);
    Req.LifetimeSeconds = 8.0f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnBreathVapor(AActor* DinoActor, FName MouthSocket)
{
    if (!DinoActor)
    {
        return -1;
    }

    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Dinosaur;
    Req.Location = DinoActor->GetActorLocation();
    Req.AttachTarget = DinoActor;
    Req.AttachSocketName = MouthSocket;
    Req.Scale = 1.0f;
    Req.LifetimeSeconds = 1.2f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

int32 UVFX_SpawnManager::SpawnCraftingSparks(FVector Location)
{
    FVFX_SpawnRequest Req;
    Req.Category = EVFX_SpawnCategory::Player;
    Req.Location = Location;
    Req.Scale = 0.6f;
    Req.LifetimeSeconds = 0.8f;
    Req.bAutoDestroy = true;
    return SpawnEffect(Req);
}

// --- State Queries ---

int32 UVFX_SpawnManager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

int32 UVFX_SpawnManager::GetActiveEffectCountByCategory(EVFX_SpawnCategory Category) const
{
    int32 Count = 0;
    for (const auto& Pair : ActiveEffects)
    {
        if (Pair.Value.Category == Category)
        {
            Count++;
        }
    }
    return Count;
}

bool UVFX_SpawnManager::IsEffectActive(int32 EffectID) const
{
    return ActiveEffects.Contains(EffectID);
}

// --- Private Helpers ---

int32 UVFX_SpawnManager::GenerateEffectID()
{
    return NextEffectID++;
}

void UVFX_SpawnManager::CleanupExpiredEffects()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    float Now = World->GetTimeSeconds();
    TArray<int32> Expired;

    for (const auto& Pair : ActiveEffects)
    {
        const FVFX_ActiveEffect& Effect = Pair.Value;
        if (Effect.LifetimeSeconds > 0.0f &&
            (Now - Effect.SpawnTime) >= Effect.LifetimeSeconds)
        {
            Expired.Add(Pair.Key);
        }
    }

    for (int32 ID : Expired)
    {
        StopEffect(ID, true);
    }
}

float UVFX_SpawnManager::GetDistanceToCamera(const FVector& Location) const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0.0f;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager)
    {
        return 0.0f;
    }

    FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    return FVector::Dist(CamLoc, Location);
}

void UVFX_SpawnManager::ApplyLODToEffect(FVFX_ActiveEffect& Effect, EVFX_LODLevel NewLOD)
{
    if (!Effect.EffectActor)
    {
        return;
    }

    // Culled = hide actor entirely
    bool bShouldBeVisible = (NewLOD != EVFX_LODLevel::Culled);
    Effect.EffectActor->SetActorHiddenInGame(!bShouldBeVisible);

    UE_LOG(LogTemp, Verbose, TEXT("[VFX_SpawnManager] Effect ID=%d LOD changed to %d (visible=%d)"),
        Effect.EffectID, (int32)NewLOD, (int32)bShouldBeVisible);
}
