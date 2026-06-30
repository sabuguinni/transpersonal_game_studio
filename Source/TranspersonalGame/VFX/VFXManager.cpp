// VFXManager.cpp
// VFX Agent #17 — Transpersonal Game Studio
// Prehistoric dinosaur survival game — physical VFX only, no spiritual/magical effects

#include "VFXManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20 Hz tick for VFX updates

    MaxActiveEffects = 64;
    LODDistance_Near = 1500.0f;
    LODDistance_Mid = 4000.0f;
    LODDistance_Far = 8000.0f;
    bVFXEnabled = true;
    bWeatherVFXActive = false;
    CurrentWeatherIntensity = 0.0f;
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[VFXManager] BeginPlay — VFX system initialised. MaxEffects=%d"), MaxActiveEffects);
}

void AVFX_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bVFXEnabled) return;

    // Tick all active effects, remove expired ones
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        FVFX_EffectEntry& Entry = ActiveEffects[i];
        Entry.ElapsedTime += DeltaTime;

        if (Entry.Duration > 0.0f && Entry.ElapsedTime >= Entry.Duration)
        {
            StopEffect(Entry.EffectID);
        }
    }
}

int32 AVFX_Manager::SpawnEffect(FVFX_SpawnRequest Request)
{
    if (!bVFXEnabled)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFXManager] VFX disabled — SpawnEffect ignored"));
        return -1;
    }

    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("[VFXManager] MaxActiveEffects (%d) reached — SpawnEffect rejected"), MaxActiveEffects);
        return -1;
    }

    // Determine LOD level based on distance from camera
    EVFX_LODLevel LOD = GetLODForDistance(Request.SpawnLocation);

    // Skip Far LOD effects if budget is tight
    if (LOD == EVFX_LODLevel::Culled)
    {
        return -1;
    }

    FVFX_EffectEntry NewEntry;
    NewEntry.EffectID = NextEffectID++;
    NewEntry.EffectType = Request.EffectType;
    NewEntry.SpawnLocation = Request.SpawnLocation;
    NewEntry.SpawnRotation = Request.SpawnRotation;
    NewEntry.Scale = Request.Scale;
    NewEntry.Duration = Request.Duration;
    NewEntry.ElapsedTime = 0.0f;
    NewEntry.LODLevel = LOD;
    NewEntry.bLooping = Request.bLooping;

    ActiveEffects.Add(NewEntry);

    UE_LOG(LogTemp, Log, TEXT("[VFXManager] SpawnEffect ID=%d Type=%d LOD=%d at (%.0f,%.0f,%.0f)"),
        NewEntry.EffectID,
        (int32)Request.EffectType,
        (int32)LOD,
        Request.SpawnLocation.X,
        Request.SpawnLocation.Y,
        Request.SpawnLocation.Z);

    return NewEntry.EffectID;
}

void AVFX_Manager::StopEffect(int32 EffectID)
{
    for (int32 i = 0; i < ActiveEffects.Num(); ++i)
    {
        if (ActiveEffects[i].EffectID == EffectID)
        {
            UE_LOG(LogTemp, Log, TEXT("[VFXManager] StopEffect ID=%d"), EffectID);
            ActiveEffects.RemoveAt(i);
            return;
        }
    }
}

void AVFX_Manager::StopAllEffects()
{
    UE_LOG(LogTemp, Log, TEXT("[VFXManager] StopAllEffects — clearing %d active effects"), ActiveEffects.Num());
    ActiveEffects.Empty();
}

int32 AVFX_Manager::SpawnCampfireEffect(FVector Location, float Duration)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::CampfireFire;
    Req.SpawnLocation = Location;
    Req.SpawnRotation = FRotator::ZeroRotator;
    Req.Scale = FVector(1.0f);
    Req.Duration = Duration;
    Req.bLooping = (Duration <= 0.0f);
    return SpawnEffect(Req);
}

int32 AVFX_Manager::SpawnDinoFootstepDust(FVector Location, FRotator Direction)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::DinoFootstepDust;
    Req.SpawnLocation = Location;
    Req.SpawnRotation = Direction;
    Req.Scale = FVector(1.5f, 1.5f, 1.0f);
    Req.Duration = 1.2f;
    Req.bLooping = false;
    return SpawnEffect(Req);
}

int32 AVFX_Manager::SpawnBloodImpact(FVector Location, FRotator SurfaceNormal)
{
    FVFX_SpawnRequest Req;
    Req.EffectType = EVFX_EffectType::BloodImpact;
    Req.SpawnLocation = Location;
    Req.SpawnRotation = SurfaceNormal;
    Req.Scale = FVector(1.0f);
    Req.Duration = 0.6f;
    Req.bLooping = false;
    return SpawnEffect(Req);
}

void AVFX_Manager::SetWeatherVFX(bool bEnable, float Intensity)
{
    bWeatherVFXActive = bEnable;
    CurrentWeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    UE_LOG(LogTemp, Log, TEXT("[VFXManager] SetWeatherVFX — Enabled=%s Intensity=%.2f"),
        bEnable ? TEXT("true") : TEXT("false"),
        CurrentWeatherIntensity);

    if (bEnable)
    {
        // Spawn rain effect at high altitude above player
        FVFX_SpawnRequest RainReq;
        RainReq.EffectType = EVFX_EffectType::Rain;
        RainReq.SpawnLocation = FVector(0.0f, 0.0f, 800.0f);
        RainReq.SpawnRotation = FRotator(-90.0f, 0.0f, 0.0f);
        RainReq.Scale = FVector(CurrentWeatherIntensity * 2.0f);
        RainReq.Duration = -1.0f; // Indefinite
        RainReq.bLooping = true;
        SpawnEffect(RainReq);
    }
    else
    {
        // Stop all rain effects
        for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
        {
            if (ActiveEffects[i].EffectType == EVFX_EffectType::Rain ||
                ActiveEffects[i].EffectType == EVFX_EffectType::Snow)
            {
                StopEffect(ActiveEffects[i].EffectID);
            }
        }
    }
}

int32 AVFX_Manager::GetActiveEffectCount() const
{
    return ActiveEffects.Num();
}

EVFX_LODLevel AVFX_Manager::GetLODForDistance(FVector EffectLocation) const
{
    // Get camera/player location
    UWorld* World = GetWorld();
    if (!World) return EVFX_LODLevel::Near;

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return EVFX_LODLevel::Near;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return EVFX_LODLevel::Near;

    float Distance = FVector::Dist(EffectLocation, PlayerPawn->GetActorLocation());

    if (Distance <= LODDistance_Near)  return EVFX_LODLevel::Near;
    if (Distance <= LODDistance_Mid)   return EVFX_LODLevel::Mid;
    if (Distance <= LODDistance_Far)   return EVFX_LODLevel::Far;
    return EVFX_LODLevel::Culled;
}
