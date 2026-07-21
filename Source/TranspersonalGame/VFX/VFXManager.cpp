// VFXManager.cpp
// Agent #17 — VFX Agent
// Full implementation of UVFX_Manager — Niagara particle spawn, LOD chain, prehistoric VFX effects

#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraShakeBase.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for LOD updates

    // LOD distance thresholds (cm)
    LODDistanceFull    = 1500.0f;
    LODDistanceMedium  = 4000.0f;
    LODDistanceCull    = 8000.0f;

    bVFXEnabled = true;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("[VFX] UVFX_Manager BeginPlay — VFX system initialised"));
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateLODChain();
}

// ─────────────────────────────────────────────────────────────────────────────
// CATEGORY 1 — ENVIRONMENT / NATURAL
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::SpawnCampfireVFX(const FVector& Location, AActor* OwnerActor)
{
    if (!bVFXEnabled || !NS_Fire_Campfire) return;

    UNiagaraComponent* FireComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Fire_Campfire, Location, FRotator::ZeroRotator,
        FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
    );

    if (FireComp && NS_Fire_Smoke)
    {
        // Spawn smoke slightly above fire origin
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), NS_Fire_Smoke, Location + FVector(0.f, 0.f, 40.f),
            FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
        );
    }

    if (FireComp && NS_Fire_Embers)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), NS_Fire_Embers, Location + FVector(0.f, 0.f, 20.f),
            FRotator::ZeroRotator, FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
        );
    }

    FVFX_SystemEntry Entry;
    Entry.SystemRef   = FireComp;
    Entry.WorldLocation = Location;
    Entry.Category    = EVFX_Category::Environment;
    Entry.CurrentLOD  = EVFX_LODLevel::Full;
    Entry.OwnerActor  = OwnerActor;
    ActiveSystems.Add(Entry);

    UE_LOG(LogTemp, Log, TEXT("[VFX] Campfire VFX spawned at %s"), *Location.ToString());
}

void UVFX_Manager::SpawnRainVFX(const FVector& Location, float Intensity)
{
    if (!bVFXEnabled || !NS_Weather_Rain) return;

    UNiagaraComponent* RainComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Weather_Rain, Location, FRotator::ZeroRotator,
        FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
    );

    if (RainComp)
    {
        RainComp->SetVariableFloat(FName("RainIntensity"), FMath::Clamp(Intensity, 0.0f, 1.0f));
    }

    UE_LOG(LogTemp, Log, TEXT("[VFX] Rain VFX spawned — intensity: %.2f"), Intensity);
}

void UVFX_Manager::SpawnDustStormVFX(const FVector& Location, const FVector& WindDirection)
{
    if (!bVFXEnabled || !NS_Weather_DustStorm) return;

    FRotator WindRot = WindDirection.Rotation();
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Weather_DustStorm, Location, WindRot,
        FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
    );

    UE_LOG(LogTemp, Log, TEXT("[VFX] Dust storm VFX spawned — wind dir: %s"), *WindDirection.ToString());
}

void UVFX_Manager::SpawnWaterfallSplashVFX(const FVector& BaseLocation)
{
    if (!bVFXEnabled || !NS_Water_Splash) return;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Water_Splash, BaseLocation, FRotator::ZeroRotator,
        FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
    );

    UE_LOG(LogTemp, Log, TEXT("[VFX] Waterfall splash VFX spawned at %s"), *BaseLocation.ToString());
}

// ─────────────────────────────────────────────────────────────────────────────
// CATEGORY 2 — DINOSAURS
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::SpawnDinosaurFootstepDust(const FVector& FootLocation, float DinoMassKg)
{
    if (!bVFXEnabled || !NS_Dino_FootstepDust) return;

    // Scale dust cloud by dino mass — T-Rex (8000kg) vs Raptor (80kg)
    float ScaleMultiplier = FMath::GetMappedRangeValueClamped(
        FVector2D(80.0f, 8000.0f),
        FVector2D(0.3f, 2.5f),
        DinoMassKg
    );

    UNiagaraComponent* DustComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Dino_FootstepDust, FootLocation, FRotator::ZeroRotator,
        FVector(ScaleMultiplier), true, true, ENCPoolMethod::AutoRelease
    );

    if (DustComp)
    {
        DustComp->SetVariableFloat(FName("ImpactForce"), DinoMassKg / 1000.0f);
    }

    UE_LOG(LogTemp, Verbose, TEXT("[VFX] Dino footstep dust at %s — mass: %.0fkg, scale: %.2f"),
        *FootLocation.ToString(), DinoMassKg, ScaleMultiplier);
}

void UVFX_Manager::SpawnDinosaurBreathVapor(const FVector& MouthLocation, const FRotator& HeadRotation, float AmbientTemperatureC)
{
    if (!bVFXEnabled || !NS_Dino_BreathVapor) return;

    // Only show breath vapor in cold environments (below 10°C)
    if (AmbientTemperatureC > 10.0f) return;

    float VaporDensity = FMath::GetMappedRangeValueClamped(
        FVector2D(-20.0f, 10.0f),
        FVector2D(1.0f, 0.1f),
        AmbientTemperatureC
    );

    UNiagaraComponent* VaporComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Dino_BreathVapor, MouthLocation, HeadRotation,
        FVector(1.0f), true, true, ENCPoolMethod::AutoRelease
    );

    if (VaporComp)
    {
        VaporComp->SetVariableFloat(FName("VaporDensity"), VaporDensity);
    }
}

void UVFX_Manager::SpawnDinosaurRoarDistortion(const FVector& MouthLocation, float RoarIntensity)
{
    if (!bVFXEnabled || !NS_Dino_RoarDistortion) return;

    UNiagaraComponent* DistortComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Dino_RoarDistortion, MouthLocation, FRotator::ZeroRotator,
        FVector(RoarIntensity), true, true, ENCPoolMethod::AutoRelease
    );

    if (DistortComp)
    {
        DistortComp->SetVariableFloat(FName("WavefrontRadius"), RoarIntensity * 500.0f);
        DistortComp->SetVariableFloat(FName("WavefrontSpeed"), 340.0f); // speed of sound m/s
    }

    // Trigger camera shake for nearby player
    TriggerCameraShakeForRoar(MouthLocation, RoarIntensity);

    UE_LOG(LogTemp, Log, TEXT("[VFX] Roar distortion VFX spawned — intensity: %.2f"), RoarIntensity);
}

void UVFX_Manager::SpawnBloodImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, float DamageAmount)
{
    if (!bVFXEnabled || !NS_Combat_Blood) return;

    FRotator ImpactRot = ImpactNormal.Rotation();
    float BloodScale = FMath::GetMappedRangeValueClamped(
        FVector2D(10.0f, 200.0f),
        FVector2D(0.5f, 2.0f),
        DamageAmount
    );

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Combat_Blood, ImpactLocation, ImpactRot,
        FVector(BloodScale), true, true, ENCPoolMethod::AutoRelease
    );

    UE_LOG(LogTemp, Verbose, TEXT("[VFX] Blood impact at %s — damage: %.1f, scale: %.2f"),
        *ImpactLocation.ToString(), DamageAmount, BloodScale);
}

// ─────────────────────────────────────────────────────────────────────────────
// CATEGORY 3 — PLAYER & COMBAT
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::SpawnWeaponImpact(const FVector& ImpactLocation, const FVector& ImpactNormal, EVFX_WeaponType WeaponType)
{
    if (!bVFXEnabled) return;

    UNiagaraSystem* TargetSystem = nullptr;
    switch (WeaponType)
    {
        case EVFX_WeaponType::Spear:
        case EVFX_WeaponType::Bow:
            TargetSystem = NS_Combat_Blood;
            break;
        case EVFX_WeaponType::Stone:
            TargetSystem = NS_Dino_FootstepDust; // Reuse dust for stone impact
            break;
        case EVFX_WeaponType::Torch:
            TargetSystem = NS_Fire_Embers;
            break;
        default:
            TargetSystem = NS_Combat_Blood;
            break;
    }

    if (TargetSystem)
    {
        FRotator ImpactRot = ImpactNormal.Rotation();
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), TargetSystem, ImpactLocation, ImpactRot,
            FVector(0.8f), true, true, ENCPoolMethod::AutoRelease
        );
    }
}

void UVFX_Manager::SpawnCraftingSparkVFX(const FVector& CraftingLocation)
{
    if (!bVFXEnabled || !NS_Crafting_Sparks) return;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        GetWorld(), NS_Crafting_Sparks, CraftingLocation, FRotator::ZeroRotator,
        FVector(0.5f), true, true, ENCPoolMethod::AutoRelease
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// CAMERA SHAKE
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::TriggerCameraShakeFromDinosaur(const FVector& DinoLocation, float DinoMassKg)
{
    if (!CameraShakeClass) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    float DistanceCm = FVector::Dist(DinoLocation, PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);
    float MaxShakeDistance = 2000.0f;

    if (DistanceCm > MaxShakeDistance) return;

    float ShakeScale = FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, MaxShakeDistance),
        FVector2D(1.0f, 0.0f),
        DistanceCm
    );

    // Scale by dino mass
    ShakeScale *= FMath::GetMappedRangeValueClamped(
        FVector2D(80.0f, 8000.0f),
        FVector2D(0.2f, 1.0f),
        DinoMassKg
    );

    PC->ClientStartCameraShake(CameraShakeClass, ShakeScale);

    UE_LOG(LogTemp, Log, TEXT("[VFX] Camera shake triggered — scale: %.2f, dist: %.0fcm"), ShakeScale, DistanceCm);
}

void UVFX_Manager::TriggerCameraShakeForRoar(const FVector& RoarLocation, float Intensity)
{
    if (!CameraShakeClass) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    float DistanceCm = FVector::Dist(RoarLocation, PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);
    float MaxRoarDistance = 5000.0f;

    if (DistanceCm > MaxRoarDistance) return;

    float ShakeScale = Intensity * FMath::GetMappedRangeValueClamped(
        FVector2D(0.0f, MaxRoarDistance),
        FVector2D(0.8f, 0.0f),
        DistanceCm
    );

    PC->ClientStartCameraShake(CameraShakeClass, ShakeScale);
}

// ─────────────────────────────────────────────────────────────────────────────
// LOD CHAIN — 3 LEVELS
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::UpdateLODChain()
{
    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC || !PC->GetPawn()) return;

    FVector PlayerLoc = PC->GetPawn()->GetActorLocation();

    for (FVFX_SystemEntry& Entry : ActiveSystems)
    {
        if (!Entry.SystemRef.IsValid()) continue;

        float Distance = FVector::Dist(PlayerLoc, Entry.WorldLocation);
        EVFX_LODLevel NewLOD;

        if (Distance <= LODDistanceFull)
            NewLOD = EVFX_LODLevel::Full;
        else if (Distance <= LODDistanceMedium)
            NewLOD = EVFX_LODLevel::Medium;
        else if (Distance <= LODDistanceCull)
            NewLOD = EVFX_LODLevel::Low;
        else
            NewLOD = EVFX_LODLevel::Culled;

        if (NewLOD != Entry.CurrentLOD)
        {
            ApplyLODToSystem(Entry, NewLOD);
            Entry.CurrentLOD = NewLOD;
        }
    }

    // Purge invalid entries
    ActiveSystems.RemoveAll([](const FVFX_SystemEntry& E) { return !E.SystemRef.IsValid(); });
}

void UVFX_Manager::ApplyLODToSystem(FVFX_SystemEntry& Entry, EVFX_LODLevel NewLOD)
{
    UNiagaraComponent* Comp = Entry.SystemRef.Get();
    if (!Comp) return;

    switch (NewLOD)
    {
        case EVFX_LODLevel::Full:
            Comp->SetPaused(false);
            Comp->SetVariableFloat(FName("SpawnRateMultiplier"), 1.0f);
            break;
        case EVFX_LODLevel::Medium:
            Comp->SetPaused(false);
            Comp->SetVariableFloat(FName("SpawnRateMultiplier"), 0.5f);
            break;
        case EVFX_LODLevel::Low:
            Comp->SetPaused(false);
            Comp->SetVariableFloat(FName("SpawnRateMultiplier"), 0.2f);
            break;
        case EVFX_LODLevel::Culled:
            Comp->SetPaused(true);
            break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// UTILITY
// ─────────────────────────────────────────────────────────────────────────────

void UVFX_Manager::SetVFXEnabled(bool bEnabled)
{
    bVFXEnabled = bEnabled;

    if (!bEnabled)
    {
        for (FVFX_SystemEntry& Entry : ActiveSystems)
        {
            if (Entry.SystemRef.IsValid())
                Entry.SystemRef->SetPaused(true);
        }
    }
    UE_LOG(LogTemp, Log, TEXT("[VFX] VFX system %s"), bEnabled ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UVFX_Manager::StopAllVFX()
{
    for (FVFX_SystemEntry& Entry : ActiveSystems)
    {
        if (Entry.SystemRef.IsValid())
            Entry.SystemRef->DeactivateImmediate();
    }
    ActiveSystems.Empty();
    UE_LOG(LogTemp, Log, TEXT("[VFX] All VFX systems stopped and cleared"));
}

int32 UVFX_Manager::GetActiveSystemCount() const
{
    int32 Count = 0;
    for (const FVFX_SystemEntry& Entry : ActiveSystems)
    {
        if (Entry.SystemRef.IsValid()) Count++;
    }
    return Count;
}
