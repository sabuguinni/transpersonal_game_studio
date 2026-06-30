#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

AVFX_Manager::AVFX_Manager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Tick every 100ms for performance

    // Default LOD distances
    LOD0_Distance = 1500.0f;
    LOD1_Distance = 4000.0f;
    LOD2_Distance = 8000.0f;
    MaxActiveEffects = 64;
    bEnableLODCulling = true;

    // Default weather state — clear day
    RainIntensity = 0.0f;
    FogDensity = 0.02f;
    bIsStorming = false;
}

void AVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: BeginPlay — VFX system initialized. Effect library size: %d"), EffectLibrary.Num());
}

void AVFX_Manager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    PruneDeadEffects();
}

UNiagaraComponent* AVFX_Manager::SpawnEffect(const FVFX_SpawnRequest& Request)
{
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: MaxActiveEffects (%d) reached — skipping spawn"), MaxActiveEffects);
        return nullptr;
    }

    const FVFX_EffectEntry* Entry = FindEffectEntry(Request.EffectType);
    if (!Entry)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: No effect entry found for type %d"), (int32)Request.EffectType);
        return nullptr;
    }

    UNiagaraSystem* NiagaraAsset = Entry->NiagaraSystem.LoadSynchronous();
    if (!NiagaraAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: NiagaraSystem not loaded for effect type %d"), (int32)Request.EffectType);
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = nullptr;

    if (Request.AttachTarget && Request.AttachSocket != NAME_None)
    {
        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            NiagaraAsset,
            Request.AttachTarget->GetRootComponent(),
            Request.AttachSocket,
            Request.Location,
            Request.Rotation,
            EAttachLocation::KeepRelativeOffset,
            Entry->bAutoDestroy
        );
    }
    else
    {
        NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            NiagaraAsset,
            Request.Location,
            Request.Rotation,
            FVector(Request.Scale),
            Entry->bAutoDestroy
        );
    }

    if (NiagaraComp)
    {
        ActiveEffects.Add(NiagaraComp);
        UE_LOG(LogTemp, Verbose, TEXT("VFX_Manager: Spawned effect type %d at %s"), (int32)Request.EffectType, *Request.Location.ToString());
    }

    return NiagaraComp;
}

UNiagaraComponent* AVFX_Manager::SpawnEffectAtLocation(EVFX_EffectType EffectType, FVector Location, float Scale)
{
    FVFX_SpawnRequest Request;
    Request.EffectType = EffectType;
    Request.Location = Location;
    Request.Rotation = FRotator::ZeroRotator;
    Request.Scale = Scale;
    return SpawnEffect(Request);
}

void AVFX_Manager::SetWeatherState(float InRainIntensity, float InFogDensity, bool bInIsStorming)
{
    RainIntensity = FMath::Clamp(InRainIntensity, 0.0f, 1.0f);
    FogDensity = FMath::Clamp(InFogDensity, 0.0f, 1.0f);
    bIsStorming = bInIsStorming;

    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Weather updated — Rain: %.2f, Fog: %.2f, Storm: %s"),
        RainIntensity, FogDensity, bIsStorming ? TEXT("YES") : TEXT("NO"));

    // Spawn rain effect if intensity > 0
    if (RainIntensity > 0.1f)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
            FVector RainLoc = PlayerLoc + FVector(0.0f, 0.0f, 800.0f);
            SpawnEffectAtLocation(EVFX_EffectType::Weather_Rain, RainLoc, RainIntensity);
        }
    }
}

void AVFX_Manager::SpawnDinoFootstepDust(FVector ImpactLocation, float DinoMassKg)
{
    // Scale dust effect based on dinosaur mass
    float Scale = FMath::Clamp(DinoMassKg / 5000.0f, 0.3f, 3.0f);

    // Large dinos (T-Rex ~8000kg) get big dust clouds
    // Small dinos (Raptor ~80kg) get small puffs
    FVFX_SpawnRequest Request;
    Request.EffectType = EVFX_EffectType::Dust_Footstep;
    Request.Location = ImpactLocation;
    Request.Rotation = FRotator::ZeroRotator;
    Request.Scale = Scale;

    SpawnEffect(Request);

    UE_LOG(LogTemp, Verbose, TEXT("VFX_Manager: Footstep dust at %s, mass=%.0fkg, scale=%.2f"),
        *ImpactLocation.ToString(), DinoMassKg, Scale);
}

void AVFX_Manager::SpawnBloodImpact(FVector HitLocation, FVector HitNormal, float DamageAmount)
{
    float Scale = FMath::Clamp(DamageAmount / 50.0f, 0.2f, 2.0f);

    FVFX_SpawnRequest Request;
    Request.EffectType = EVFX_EffectType::Dino_BloodImpact;
    Request.Location = HitLocation;
    Request.Rotation = HitNormal.Rotation();
    Request.Scale = Scale;

    SpawnEffect(Request);
}

void AVFX_Manager::SpawnCampfireEffect(FVector CampfireLocation)
{
    // Fire effect
    SpawnEffectAtLocation(EVFX_EffectType::Fire_Campfire, CampfireLocation, 1.0f);

    // Ambient insects around campfire at night
    FVector InsectLoc = CampfireLocation + FVector(0.0f, 0.0f, 100.0f);
    SpawnEffectAtLocation(EVFX_EffectType::Ambient_Insects, InsectLoc, 0.5f);

    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Campfire VFX spawned at %s"), *CampfireLocation.ToString());
}

void AVFX_Manager::StopAllEffects()
{
    for (UNiagaraComponent* Comp : ActiveEffects)
    {
        if (IsValid(Comp))
        {
            Comp->DeactivateImmediate();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: All effects stopped"));
}

int32 AVFX_Manager::GetActiveEffectCount() const
{
    int32 Count = 0;
    for (const UNiagaraComponent* Comp : ActiveEffects)
    {
        if (IsValid(Comp) && Comp->IsActive())
        {
            Count++;
        }
    }
    return Count;
}

void AVFX_Manager::DebugSpawnAllEffects()
{
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: DEBUG — Spawning all effect types for preview"));

    TArray<EVFX_EffectType> AllTypes = {
        EVFX_EffectType::Fire_Campfire,
        EVFX_EffectType::Dust_Footstep,
        EVFX_EffectType::Weather_Rain,
        EVFX_EffectType::Dino_Breath,
        EVFX_EffectType::Dino_BloodImpact,
        EVFX_EffectType::Combat_SpearImpact,
        EVFX_EffectType::Ambient_Insects,
        EVFX_EffectType::Ambient_VolcanicAsh
    };

    FVector BaseLocation = GetActorLocation();
    float Spacing = 300.0f;

    for (int32 i = 0; i < AllTypes.Num(); i++)
    {
        FVector SpawnLoc = BaseLocation + FVector(i * Spacing, 0.0f, 0.0f);
        SpawnEffectAtLocation(AllTypes[i], SpawnLoc, 1.0f);

#if WITH_EDITOR
        DrawDebugSphere(GetWorld(), SpawnLoc, 50.0f, 12, FColor::Green, false, 5.0f);
#endif
    }
}

const FVFX_EffectEntry* AVFX_Manager::FindEffectEntry(EVFX_EffectType EffectType) const
{
    for (const FVFX_EffectEntry& Entry : EffectLibrary)
    {
        if (Entry.EffectType == EffectType)
        {
            return &Entry;
        }
    }
    return nullptr;
}

void AVFX_Manager::PruneDeadEffects()
{
    ActiveEffects.RemoveAll([](UNiagaraComponent* Comp) {
        return !IsValid(Comp) || !Comp->IsActive();
    });
}

float AVFX_Manager::GetLODScaleForDistance(float Distance) const
{
    if (!bEnableLODCulling) return 1.0f;

    if (Distance <= LOD0_Distance) return 1.0f;
    if (Distance <= LOD1_Distance) return 0.6f;
    if (Distance <= LOD2_Distance) return 0.3f;
    return 0.0f; // Beyond LOD2 — cull
}
