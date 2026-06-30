// NiagaraVFXLibrary.cpp
// VFX Agent #17 — Transpersonal Game Studio
// Full implementation of the Niagara VFX library for prehistoric survival game.
// Handles campfire, dinosaur footsteps, weather, combat impacts, and world VFX.

#include "NiagaraVFXLibrary.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UVFX_NiagaraLibrary — Static Spawn Helpers
// ============================================================

UVFX_NiagaraLibrary::UVFX_NiagaraLibrary()
{
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAtLocation(
    UObject* WorldContext,
    UNiagaraSystem* NiagaraSystem,
    const FVFX_SpawnParams& Params)
{
    if (!WorldContext || !NiagaraSystem)
    {
        return nullptr;
    }

    UWorld* World = GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull);
    if (!World)
    {
        return nullptr;
    }

    // LOD distance check — cull if beyond threshold
    APlayerController* PC = World->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        float DistSq = FVector::DistSquared(PC->GetPawn()->GetActorLocation(), Params.Location);
        float CullDistSq = FVFX_LODThresholds::GetCullDistance(Params.Category) * FVFX_LODThresholds::GetCullDistance(Params.Category);
        if (DistSq > CullDistSq)
        {
            return nullptr;
        }
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        World,
        NiagaraSystem,
        Params.Location,
        Params.Rotation,
        Params.Scale,
        true,  // bAutoDestroy
        !Params.bLooping,  // bAutoActivate
        ENCPoolMethod::AutoRelease
    );

    return NiagaraComp;
}

UNiagaraComponent* UVFX_NiagaraLibrary::SpawnVFXAttached(
    UNiagaraSystem* NiagaraSystem,
    USceneComponent* AttachToComponent,
    FName AttachPointName,
    const FVFX_SpawnParams& Params)
{
    if (!NiagaraSystem || !AttachToComponent)
    {
        return nullptr;
    }

    UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
        NiagaraSystem,
        AttachToComponent,
        AttachPointName,
        Params.Location,
        Params.Rotation,
        EAttachLocation::KeepRelativeOffset,
        true,  // bAutoDestroy
        true,  // bAutoActivate
        ENCPoolMethod::AutoRelease
    );

    return NiagaraComp;
}

EVFX_LODLevel UVFX_NiagaraLibrary::CalculateLODLevel(
    const FVector& VFXLocation,
    const FVector& CameraLocation,
    EVFX_Category Category)
{
    float Distance = FVector::Dist(VFXLocation, CameraLocation);
    return FVFX_LODThresholds::GetLODForDistance(Distance, Category);
}

// ============================================================
// UVFX_WeatherController
// ============================================================

UVFX_WeatherController::UVFX_WeatherController()
    : CurrentWeatherType(EVFX_WeatherType::Clear)
    , WeatherIntensity(0.0f)
    , bWeatherActive(false)
    , RainComponent(nullptr)
    , SnowComponent(nullptr)
    , FogComponent(nullptr)
    , StormComponent(nullptr)
    , RainSystem(nullptr)
    , SnowSystem(nullptr)
    , FogSystem(nullptr)
    , StormSystem(nullptr)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 Hz weather update
}

void UVFX_WeatherController::BeginPlay()
{
    Super::BeginPlay();
}

void UVFX_WeatherController::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bWeatherActive)
    {
        UpdateWeatherIntensity(DeltaTime);
    }
}

void UVFX_WeatherController::SetWeather(EVFX_WeatherType NewWeather, float Intensity)
{
    StopCurrentWeather();

    CurrentWeatherType = NewWeather;
    WeatherIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    bWeatherActive = true;

    StartWeatherEffect(NewWeather, WeatherIntensity);
}

void UVFX_WeatherController::StopCurrentWeather()
{
    bWeatherActive = false;

    if (RainComponent) { RainComponent->Deactivate(); RainComponent = nullptr; }
    if (SnowComponent) { SnowComponent->Deactivate(); SnowComponent = nullptr; }
    if (FogComponent)  { FogComponent->Deactivate();  FogComponent = nullptr;  }
    if (StormComponent){ StormComponent->Deactivate(); StormComponent = nullptr; }
}

void UVFX_WeatherController::StartWeatherEffect(EVFX_WeatherType WeatherType, float Intensity)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UNiagaraSystem* TargetSystem = nullptr;
    UNiagaraComponent** TargetComp = nullptr;

    switch (WeatherType)
    {
        case EVFX_WeatherType::Rain:
            TargetSystem = RainSystem;
            TargetComp = &RainComponent;
            break;
        case EVFX_WeatherType::Snow:
            TargetSystem = SnowSystem;
            TargetComp = &SnowComponent;
            break;
        case EVFX_WeatherType::Fog:
            TargetSystem = FogSystem;
            TargetComp = &FogComponent;
            break;
        case EVFX_WeatherType::Storm:
            TargetSystem = StormSystem;
            TargetComp = &StormComponent;
            break;
        default:
            return;
    }

    if (TargetSystem && TargetComp)
    {
        *TargetComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TargetSystem,
            Owner->GetRootComponent(),
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false,  // bAutoDestroy = false (looping)
            true,
            ENCPoolMethod::None
        );

        if (*TargetComp)
        {
            (*TargetComp)->SetFloatParameter(FName("Intensity"), Intensity);
        }
    }
}

void UVFX_WeatherController::UpdateWeatherIntensity(float DeltaTime)
{
    UNiagaraComponent* ActiveComp = nullptr;
    switch (CurrentWeatherType)
    {
        case EVFX_WeatherType::Rain:  ActiveComp = RainComponent;  break;
        case EVFX_WeatherType::Snow:  ActiveComp = SnowComponent;  break;
        case EVFX_WeatherType::Fog:   ActiveComp = FogComponent;   break;
        case EVFX_WeatherType::Storm: ActiveComp = StormComponent; break;
        default: break;
    }

    if (ActiveComp)
    {
        ActiveComp->SetFloatParameter(FName("Intensity"), WeatherIntensity);
    }
}

// ============================================================
// UVFX_DinosaurImpactManager
// ============================================================

UVFX_DinosaurImpactManager::UVFX_DinosaurImpactManager()
    : FootstepDustSystem(nullptr)
    , BreathVaporSystem(nullptr)
    , BloodImpactSystem(nullptr)
    , RoarDistortionSystem(nullptr)
    , FootstepDustScale(1.0f)
    , bEnableBreathVapor(true)
    , bEnableBloodEffects(true)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UVFX_DinosaurImpactManager::BeginPlay()
{
    Super::BeginPlay();
}

void UVFX_DinosaurImpactManager::SpawnFootstepDust(
    const FVector& FootLocation,
    const FVector& FootNormal,
    float DinosaurMass)
{
    if (!FootstepDustSystem) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    // Scale dust by dinosaur mass (larger dinos = bigger dust cloud)
    float DustScale = FMath::Clamp(DinosaurMass / 5000.0f, 0.3f, 3.0f) * FootstepDustScale;

    FVFX_SpawnParams Params;
    Params.Location = FootLocation;
    Params.Rotation = FootNormal.Rotation();
    Params.Scale = FVector(DustScale);
    Params.Category = EVFX_Category::Dinosaur;
    Params.bLooping = false;

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(),
        FootstepDustSystem,
        FootLocation,
        FootNormal.Rotation(),
        FVector(DustScale),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );
}

void UVFX_DinosaurImpactManager::SpawnBreathVapor(
    USceneComponent* MouthComponent,
    float BreathTemperature)
{
    if (!BreathVaporSystem || !bEnableBreathVapor || !MouthComponent) return;

    // Only show breath vapor in cold environments (temperature < 10°C)
    if (BreathTemperature > 10.0f) return;

    UNiagaraFunctionLibrary::SpawnSystemAttached(
        BreathVaporSystem,
        MouthComponent,
        FName("MouthSocket"),
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget,
        true,
        true,
        ENCPoolMethod::AutoRelease
    );
}

void UVFX_DinosaurImpactManager::SpawnBloodImpact(
    const FVector& ImpactLocation,
    const FVector& ImpactNormal,
    float DamageAmount)
{
    if (!BloodImpactSystem || !bEnableBloodEffects) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float BloodScale = FMath::Clamp(DamageAmount / 50.0f, 0.5f, 2.5f);

    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(),
        BloodImpactSystem,
        ImpactLocation,
        ImpactNormal.Rotation(),
        FVector(BloodScale),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );
}

void UVFX_DinosaurImpactManager::SpawnRoarDistortion(
    const FVector& MouthLocation,
    float RoarIntensity)
{
    if (!RoarDistortionSystem) return;

    AActor* Owner = GetOwner();
    if (!Owner) return;

    UNiagaraComponent* Comp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
        Owner->GetWorld(),
        RoarDistortionSystem,
        MouthLocation,
        FRotator::ZeroRotator,
        FVector(RoarIntensity),
        true,
        true,
        ENCPoolMethod::AutoRelease
    );

    if (Comp)
    {
        Comp->SetFloatParameter(FName("RoarIntensity"), RoarIntensity);
    }
}

// ============================================================
// UVFX_CampfireController
// ============================================================

UVFX_CampfireController::UVFX_CampfireController()
    : FireSystem(nullptr)
    , SmokeSystem(nullptr)
    , EmbersSystem(nullptr)
    , FireComponent(nullptr)
    , SmokeComponent(nullptr)
    , EmbersComponent(nullptr)
    , FireIntensity(1.0f)
    , bIsLit(false)
    , FireLight(nullptr)
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20 Hz for fire flicker
}

void UVFX_CampfireController::BeginPlay()
{
    Super::BeginPlay();

    // Find or create point light for fire illumination
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FireLight = Owner->FindComponentByClass<UPointLightComponent>();
        if (FireLight)
        {
            FireLight->SetIntensity(0.0f); // Start unlit
            FireLight->SetLightColor(FLinearColor(1.0f, 0.45f, 0.1f, 1.0f));
            FireLight->SetAttenuationRadius(600.0f);
        }
    }
}

void UVFX_CampfireController::TickComponent(
    float DeltaTime,
    ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsLit && FireLight)
    {
        // Fire flicker simulation using sine wave + noise
        float Time = GetWorld()->GetTimeSeconds();
        float Flicker = FMath::Sin(Time * 7.3f) * 0.15f
                      + FMath::Sin(Time * 13.7f) * 0.08f
                      + FMath::Sin(Time * 3.1f) * 0.05f;
        float BaseIntensity = 2500.0f * FireIntensity;
        FireLight->SetIntensity(BaseIntensity * (1.0f + Flicker));
    }
}

void UVFX_CampfireController::LightFire(float Intensity)
{
    if (bIsLit) return;

    bIsLit = true;
    FireIntensity = FMath::Clamp(Intensity, 0.1f, 2.0f);

    AActor* Owner = GetOwner();
    if (!Owner) return;

    USceneComponent* Root = Owner->GetRootComponent();

    if (FireSystem)
    {
        FireComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            FireSystem, Root, NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false, true, ENCPoolMethod::None
        );
        if (FireComponent)
            FireComponent->SetFloatParameter(FName("FireIntensity"), FireIntensity);
    }

    if (SmokeSystem)
    {
        SmokeComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            SmokeSystem, Root, NAME_None,
            FVector(0, 0, 30.0f), FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false, true, ENCPoolMethod::None
        );
    }

    if (EmbersSystem)
    {
        EmbersComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            EmbersSystem, Root, NAME_None,
            FVector::ZeroVector, FRotator::ZeroRotator,
            EAttachLocation::KeepRelativeOffset,
            false, true, ENCPoolMethod::None
        );
    }

    if (FireLight)
    {
        FireLight->SetIntensity(2500.0f * FireIntensity);
    }
}

void UVFX_CampfireController::ExtinguishFire()
{
    if (!bIsLit) return;

    bIsLit = false;

    if (FireComponent)  { FireComponent->Deactivate();  FireComponent = nullptr; }
    if (SmokeComponent) { SmokeComponent->Deactivate(); SmokeComponent = nullptr; }
    if (EmbersComponent){ EmbersComponent->Deactivate(); EmbersComponent = nullptr; }

    if (FireLight)
    {
        FireLight->SetIntensity(0.0f);
    }
}

void UVFX_CampfireController::SetFireIntensity(float NewIntensity)
{
    FireIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);

    if (FireComponent)
    {
        FireComponent->SetFloatParameter(FName("FireIntensity"), FireIntensity);
    }
}

// ============================================================
// FVFX_LODThresholds — Static LOD distance helpers
// ============================================================

float FVFX_LODThresholds::GetCullDistance(EVFX_Category Category)
{
    switch (Category)
    {
        case EVFX_Category::Environment: return 8000.0f;
        case EVFX_Category::Dinosaur:    return 6000.0f;
        case EVFX_Category::Combat:      return 4000.0f;
        case EVFX_Category::World:       return 12000.0f;
        case EVFX_Category::Weather:     return 20000.0f;
        default:                         return 5000.0f;
    }
}

EVFX_LODLevel FVFX_LODThresholds::GetLODForDistance(float Distance, EVFX_Category Category)
{
    float CullDist = GetCullDistance(Category);

    if (Distance > CullDist)              return EVFX_LODLevel::Culled;
    if (Distance > CullDist * 0.75f)      return EVFX_LODLevel::Low;
    if (Distance > CullDist * 0.4f)       return EVFX_LODLevel::Medium;
    return EVFX_LODLevel::High;
}
