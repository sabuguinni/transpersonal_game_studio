#include "VFXManager.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // Check every 0.5s for cleanup

    // LOD defaults (cm)
    LOD_HighMaxDistance   = 1500.0f;
    LOD_MediumMaxDistance = 4000.0f;
    LOD_LowMaxDistance    = 8000.0f;
    MaxActiveVFXSystems   = 32;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    ActiveVFXComponents.Empty();
    ActiveWeatherComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: BeginPlay — VFX system initialized. LOD chain: High=%.0fcm, Medium=%.0fcm, Low=%.0fcm"),
        LOD_HighMaxDistance, LOD_MediumMaxDistance, LOD_LowMaxDistance);
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    CleanupFinishedVFX();
}

// ============================================================
// SPAWN FUNCTIONS
// ============================================================

UNiagaraComponent* UVFX_Manager::SpawnVFX_Campfire(const FVector& Location, float Scale)
{
    UNiagaraComponent* FireComp = SpawnNiagaraAtLocation(NS_Fire_Campfire, Location, FRotator::ZeroRotator, Scale, false);
    // Also spawn smoke slightly above
    FVector SmokeLocation = Location + FVector(0.0f, 0.0f, 40.0f);
    SpawnNiagaraAtLocation(NS_Fire_CampfireSmoke, SmokeLocation, FRotator::ZeroRotator, Scale * 0.8f, false);
    // Embers at fire level
    SpawnNiagaraAtLocation(NS_Fire_CampfireEmbers, Location, FRotator::ZeroRotator, Scale, false);
    return FireComp;
}

UNiagaraComponent* UVFX_Manager::SpawnVFX_DinoFootstep(const FVector& Location, float DinoMassKg)
{
    // Scale dust based on dinosaur mass — heavier dino = bigger dust cloud
    // T-Rex ~6000kg → scale 1.0, Raptor ~80kg → scale 0.2
    float DustScale = FMath::Clamp(DinoMassKg / 6000.0f, 0.1f, 2.0f);
    UNiagaraComponent* DustComp = SpawnNiagaraAtLocation(NS_Dino_FootstepDust, Location, FRotator::ZeroRotator, DustScale, true);
    if (DustComp)
    {
        // Pass mass as Niagara user parameter for particle count scaling
        DustComp->SetFloatParameter(TEXT("DinoMassKg"), DinoMassKg);
        DustComp->SetFloatParameter(TEXT("ImpactScale"), DustScale);
    }
    return DustComp;
}

UNiagaraComponent* UVFX_Manager::SpawnVFX_BloodImpact(const FVector& Location, const FVector& ImpactNormal, float DamageAmount)
{
    // Scale blood splatter based on damage
    float BloodScale = FMath::Clamp(DamageAmount / 50.0f, 0.2f, 3.0f);
    FRotator ImpactRot = ImpactNormal.Rotation();
    UNiagaraComponent* BloodComp = SpawnNiagaraAtLocation(NS_Combat_BloodSplatter, Location, ImpactRot, BloodScale, true);
    if (BloodComp)
    {
        BloodComp->SetFloatParameter(TEXT("DamageAmount"), DamageAmount);
        BloodComp->SetVectorParameter(TEXT("ImpactNormal"), ImpactNormal);
    }
    return BloodComp;
}

UNiagaraComponent* UVFX_Manager::SpawnVFX_WeaponImpact(const FVector& Location, const FVector& ImpactNormal, bool bOnDinosaur)
{
    FRotator ImpactRot = ImpactNormal.Rotation();
    UNiagaraComponent* ImpactComp = nullptr;
    if (bOnDinosaur)
    {
        // On dinosaur: blood + flesh impact
        ImpactComp = SpawnNiagaraAtLocation(NS_Dino_BloodImpact, Location, ImpactRot, 1.0f, true);
    }
    else
    {
        // On rock/ground: rock/dirt chips
        ImpactComp = SpawnNiagaraAtLocation(NS_Combat_RockImpact, Location, ImpactRot, 1.0f, true);
    }
    return ImpactComp;
}

void UVFX_Manager::SetWeatherVFX(EVFX_Category WeatherType, float Intensity)
{
    StopAllWeatherVFX();

    UWorld* World = GetWorld();
    if (!World) return;

    Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector WeatherLocation = Owner->GetActorLocation() + FVector(0.0f, 0.0f, 1000.0f);
    UNiagaraComponent* WeatherComp = nullptr;

    switch (WeatherType)
    {
        case EVFX_Category::Weather:
            // Default to rain
            WeatherComp = SpawnNiagaraAtLocation(NS_Weather_Rain, WeatherLocation, FRotator::ZeroRotator, Intensity * 2.0f, false);
            break;
        default:
            break;
    }

    if (WeatherComp)
    {
        WeatherComp->SetFloatParameter(TEXT("Intensity"), Intensity);
        ActiveWeatherComponents.Add(WeatherComp);
        UE_LOG(LogTemp, Log, TEXT("VFX_Manager: Weather VFX started — Intensity=%.2f"), Intensity);
    }
}

void UVFX_Manager::StopAllWeatherVFX()
{
    for (UNiagaraComponent* Comp : ActiveWeatherComponents)
    {
        if (IsValid(Comp))
        {
            Comp->DeactivateImmediate();
            Comp->DestroyComponent();
        }
    }
    ActiveWeatherComponents.Empty();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: All weather VFX stopped."));
}

EVFX_LODLevel UVFX_Manager::GetLODLevelForDistance(float DistanceCm) const
{
    if (DistanceCm <= LOD_HighMaxDistance)   return EVFX_LODLevel::High;
    if (DistanceCm <= LOD_MediumMaxDistance) return EVFX_LODLevel::Medium;
    if (DistanceCm <= LOD_LowMaxDistance)    return EVFX_LODLevel::Low;
    return EVFX_LODLevel::Culled;
}

int32 UVFX_Manager::GetActiveVFXCount() const
{
    int32 Count = 0;
    for (const UNiagaraComponent* Comp : ActiveVFXComponents)
    {
        if (IsValid(Comp) && Comp->IsActive()) Count++;
    }
    return Count;
}

void UVFX_Manager::ClearAllVFX()
{
    for (UNiagaraComponent* Comp : ActiveVFXComponents)
    {
        if (IsValid(Comp))
        {
            Comp->DeactivateImmediate();
            Comp->DestroyComponent();
        }
    }
    ActiveVFXComponents.Empty();
    StopAllWeatherVFX();
    UE_LOG(LogTemp, Log, TEXT("VFX_Manager: All VFX cleared."));
}

// ============================================================
// PRIVATE HELPERS
// ============================================================

void UVFX_Manager::CleanupFinishedVFX()
{
    // Remove invalid or finished components from tracking array
    ActiveVFXComponents.RemoveAll([](UNiagaraComponent* Comp)
    {
        return !IsValid(Comp) || !Comp->IsActive();
    });
}

UNiagaraComponent* UVFX_Manager::SpawnNiagaraAtLocation(
    TSoftObjectPtr<UNiagaraSystem> NiagaraAsset,
    const FVector& Location,
    const FRotator& Rotation,
    float Scale,
    bool bAutoDestroy)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Budget check — prevent VFX overload
    if (GetActiveVFXCount() >= MaxActiveVFXSystems)
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: Active VFX budget exceeded (%d/%d). Skipping spawn."),
            GetActiveVFXCount(), MaxActiveVFXSystems);
        return nullptr;
    }

    // Load the Niagara system (sync load — assets should be pre-loaded in production)
    UNiagaraSystem* LoadedSystem = NiagaraAsset.LoadSynchronous();
    if (!LoadedSystem)
    {
        // Asset not yet assigned — log but don't crash
        UE_LOG(LogTemp, Warning, TEXT("VFX_Manager: NiagaraSystem asset not loaded for spawn at (%.0f, %.0f, %.0f). Assign asset in editor."),
            Location.X, Location.Y, Location.Z);
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        LoadedSystem,
        Location,
        Rotation,
        FVector(Scale),
        bAutoDestroy,
        true,  // bAutoActivate
        ENCPoolMethod::None
    );

    if (NiagaraComp)
    {
        ActiveVFXComponents.Add(NiagaraComp);
        UE_LOG(LogTemp, Verbose, TEXT("VFX_Manager: Spawned %s at (%.0f, %.0f, %.0f) scale=%.2f"),
            *LoadedSystem->GetName(), Location.X, Location.Y, Location.Z, Scale);
    }

    return NiagaraComp;
}
