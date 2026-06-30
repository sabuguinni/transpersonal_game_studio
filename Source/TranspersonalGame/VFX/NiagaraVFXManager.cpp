#include "VFX/NiagaraVFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/SceneComponent.h"

// ============================================================
// Constructor
// ============================================================
UNiagaraVFXManager::UNiagaraVFXManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for LOD updates
    ActiveEffectCount = 0;
}

// ============================================================
// BeginPlay
// ============================================================
void UNiagaraVFXManager::BeginPlay()
{
    Super::BeginPlay();
    ActiveLoopingEffects.Empty();
}

// ============================================================
// TickComponent — prune destroyed looping effects, update count
// ============================================================
void UNiagaraVFXManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Remove null or completed looping effects from tracking array
    ActiveLoopingEffects.RemoveAll([](const TObjectPtr<UNiagaraComponent>& Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });

    ActiveEffectCount = ActiveLoopingEffects.Num();
}

// ============================================================
// ComputeLODTier — distance from player camera to effect
// ============================================================
EVFX_LODTier UNiagaraVFXManager::ComputeLODTier(const FVector& EffectLocation) const
{
    UWorld* World = GetWorld();
    if (!World) return EVFX_LODTier::High;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC || !PC->PlayerCameraManager) return EVFX_LODTier::High;

    const FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
    const float DistSq = FVector::DistSquared(CamLoc, EffectLocation);

    const float D0 = LODDistances.HighToMedium;
    const float D1 = LODDistances.MediumToLow;
    const float D2 = LODDistances.LowToCull;

    if (DistSq > D2 * D2) return EVFX_LODTier::Culled;
    if (DistSq > D1 * D1) return EVFX_LODTier::Low;
    if (DistSq > D0 * D0) return EVFX_LODTier::Medium;
    return EVFX_LODTier::High;
}

// ============================================================
// ResolveNiagaraSystem — pick correct NS asset for effect + LOD
// (LOD Medium/Low fall back to same asset with reduced spawn rate
//  via Niagara scalability — asset assignment is the same NS)
// ============================================================
UNiagaraSystem* UNiagaraVFXManager::ResolveNiagaraSystem(EVFX_EffectType EffectType, EVFX_LODTier LOD) const
{
    if (LOD == EVFX_LODTier::Culled) return nullptr;

    switch (EffectType)
    {
        case EVFX_EffectType::CampfireFlame:      return NS_CampfireFlame;
        case EVFX_EffectType::CampfireSmoke:      return NS_CampfireSmoke;
        case EVFX_EffectType::CampfireEmbers:     return NS_CampfireEmbers;
        case EVFX_EffectType::PlayerFootstepDust: return NS_PlayerFootstepDust;
        case EVFX_EffectType::DinoFootstepDust:   return NS_DinoFootstepDust;
        case EVFX_EffectType::TRexFootstepDust:   return NS_TRexFootstepDust;
        case EVFX_EffectType::BloodSplatter:      return NS_BloodSplatter;
        case EVFX_EffectType::BloodDrip:          return NS_BloodSplatter; // reuse splatter at low scale
        case EVFX_EffectType::RainSplash:         return NS_RainSplash;
        case EVFX_EffectType::VolcanicAsh:        return NS_VolcanicAsh;
        case EVFX_EffectType::BreathVapour:       return NS_BreathVapour;
        case EVFX_EffectType::SparkCraft:         return NS_SparkCraft;
        case EVFX_EffectType::SmokeCook:          return NS_CampfireSmoke; // reuse smoke
        default:                                  return nullptr;
    }
}

// ============================================================
// EvictOldestEffect — remove oldest looping effect when pool full
// ============================================================
void UNiagaraVFXManager::EvictOldestEffect()
{
    if (ActiveLoopingEffects.Num() == 0) return;

    UNiagaraComponent* Oldest = ActiveLoopingEffects[0];
    if (IsValid(Oldest))
    {
        Oldest->DeactivateImmediate();
        Oldest->DestroyComponent();
    }
    ActiveLoopingEffects.RemoveAt(0);
}

// ============================================================
// SpawnEffect — core spawn function
// ============================================================
UNiagaraComponent* UNiagaraVFXManager::SpawnEffect(const FVFX_SpawnRequest& Request)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // LOD check
    EVFX_LODTier LOD = ComputeLODTier(Request.Location);
    UNiagaraSystem* NS = ResolveNiagaraSystem(Request.EffectType, LOD);
    if (!NS) return nullptr; // culled or no asset assigned

    // Pool management for looping effects
    if (Request.bLooping && ActiveLoopingEffects.Num() >= MaxActiveEffects)
    {
        EvictOldestEffect();
    }

    UNiagaraComponent* Comp = nullptr;

    if (Request.AttachTarget && IsValid(Request.AttachTarget))
    {
        // Attached spawn
        Comp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NS,
            Request.AttachTarget,
            NAME_None,
            Request.Location,
            Request.Rotation,
            EAttachLocation::KeepWorldPosition,
            !Request.bLooping // auto-destroy if one-shot
        );
    }
    else
    {
        // World-space spawn
        Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            World,
            NS,
            Request.Location,
            Request.Rotation,
            Request.Scale,
            true,               // auto-activate
            !Request.bLooping   // auto-destroy if one-shot
        );
    }

    if (Comp && Request.bLooping)
    {
        ActiveLoopingEffects.Add(Comp);
        ActiveEffectCount = ActiveLoopingEffects.Num();
    }

    return Comp;
}

// ============================================================
// SpawnCampfire — flame + smoke + embers at location
// ============================================================
void UNiagaraVFXManager::SpawnCampfire(FVector Location)
{
    // Flame — looping, close to ground
    FVFX_SpawnRequest FlameReq;
    FlameReq.EffectType = EVFX_EffectType::CampfireFlame;
    FlameReq.Location = Location;
    FlameReq.Scale = FVector(1.0f);
    FlameReq.bLooping = true;
    SpawnEffect(FlameReq);

    // Smoke — looping, offset upward
    FVFX_SpawnRequest SmokeReq;
    SmokeReq.EffectType = EVFX_EffectType::CampfireSmoke;
    SmokeReq.Location = Location + FVector(0, 0, 30.0f);
    SmokeReq.Scale = FVector(1.2f);
    SmokeReq.bLooping = true;
    SpawnEffect(SmokeReq);

    // Embers — looping, same base location
    FVFX_SpawnRequest EmbersReq;
    EmbersReq.EffectType = EVFX_EffectType::CampfireEmbers;
    EmbersReq.Location = Location + FVector(0, 0, 10.0f);
    EmbersReq.Scale = FVector(0.8f);
    EmbersReq.bLooping = true;
    SpawnEffect(EmbersReq);
}

// ============================================================
// SpawnPlayerFootstep — small dust puff, speed-scaled
// ============================================================
void UNiagaraVFXManager::SpawnPlayerFootstep(FVector Location, float SpeedScale)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::PlayerFootstepDust;
    Req.Location = Location;
    Req.Scale = FVector(FMath::Clamp(SpeedScale * 0.5f, 0.3f, 1.2f));
    Req.bLooping = false;
    SpawnEffect(Req);
}

// ============================================================
// SpawnTRexFootstep — large dust cloud, mass-scaled
// ============================================================
void UNiagaraVFXManager::SpawnTRexFootstep(FVector Location, float MassScale)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::TRexFootstepDust;
    Req.Location = Location;
    Req.Scale = FVector(FMath::Clamp(MassScale * 2.0f, 1.5f, 4.0f));
    Req.bLooping = false;
    SpawnEffect(Req);
}

// ============================================================
// SpawnBloodSplatter — damage-scaled blood at impact point
// ============================================================
void UNiagaraVFXManager::SpawnBloodSplatter(FVector Location, FVector ImpactNormal, float DamageAmount)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::BloodSplatter;
    Req.Location = Location;
    // Orient splatter along impact normal
    Req.Rotation = ImpactNormal.Rotation();
    // Scale by damage: 10 damage = scale 0.5, 100 damage = scale 2.0
    const float ScaleVal = FMath::Clamp(DamageAmount / 50.0f, 0.3f, 3.0f);
    Req.Scale = FVector(ScaleVal);
    Req.bLooping = false;
    SpawnEffect(Req);
}

// ============================================================
// SpawnBreathVapour — cold biome breath, attached to mouth socket
// ============================================================
void UNiagaraVFXManager::SpawnBreathVapour(USceneComponent* MouthSocket)
{
    if (!MouthSocket) return;

    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::BreathVapour;
    Req.Location = MouthSocket->GetComponentLocation();
    Req.Rotation = MouthSocket->GetComponentRotation();
    Req.Scale = FVector(0.6f);
    Req.bLooping = true;
    Req.AttachTarget = MouthSocket;
    SpawnEffect(Req);
}

// ============================================================
// StopEffect — deactivate and destroy a looping component
// ============================================================
void UNiagaraVFXManager::StopEffect(UNiagaraComponent* NiagaraComp)
{
    if (!IsValid(NiagaraComp)) return;

    NiagaraComp->DeactivateImmediate();
    ActiveLoopingEffects.Remove(NiagaraComp);
    NiagaraComp->DestroyComponent();
    ActiveEffectCount = ActiveLoopingEffects.Num();
}
