#include "VFXManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"

UVFX_Manager::UVFX_Manager()
{
    PrimaryComponentTick.bCanEverTick = true;
    MaxActiveEffects = 50;
    bAutoCleanupEffects = true;
}

void UVFX_Manager::BeginPlay()
{
    Super::BeginPlay();
    InitializeEffectLibrary();
}

void UVFX_Manager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoCleanupEffects)
    {
        CleanupFinishedEffects();
    }
}

void UVFX_Manager::InitializeEffectLibrary()
{
    // Register basic particle effects using engine defaults
    RegisterEffectSystem(EVFX_EffectType::DustCloud, "/Engine/VFX/P_Dust");
    RegisterEffectSystem(EVFX_EffectType::CampfireSmoke, "/Engine/VFX/P_Smoke");
    RegisterEffectSystem(EVFX_EffectType::WaterSplash, "/Engine/VFX/P_Water_Splash");
    RegisterEffectSystem(EVFX_EffectType::BloodSplatter, "/Engine/VFX/P_Blood");
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Manager initialized with %d effect types"), EffectLibrary.Num());
}

void UVFX_Manager::RegisterEffectSystem(EVFX_EffectType EffectType, const FString& ParticleSystemPath)
{
    UParticleSystem* ParticleSystem = LoadObject<UParticleSystem>(nullptr, *ParticleSystemPath);
    if (ParticleSystem)
    {
        EffectLibrary.Add(EffectType, ParticleSystem);
        UE_LOG(LogTemp, Log, TEXT("Registered VFX: %s"), *ParticleSystemPath);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load VFX: %s"), *ParticleSystemPath);
    }
}

UParticleSystemComponent* UVFX_Manager::SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation, FVector Scale)
{
    if (!EffectLibrary.Contains(EffectType))
    {
        UE_LOG(LogTemp, Warning, TEXT("VFX effect type not found in library"));
        return nullptr;
    }

    UParticleSystem* ParticleSystem = EffectLibrary[EffectType];
    if (!ParticleSystem)
    {
        return nullptr;
    }

    // Limit active effects for performance
    if (ActiveEffects.Num() >= MaxActiveEffects)
    {
        CleanupFinishedEffects();
    }

    UParticleSystemComponent* EffectComponent = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(), 
        ParticleSystem, 
        Location, 
        Rotation, 
        Scale, 
        true
    );

    if (EffectComponent)
    {
        ActiveEffects.Add(EffectComponent);
        UE_LOG(LogTemp, Log, TEXT("Spawned VFX effect at location: %s"), *Location.ToString());
    }

    return EffectComponent;
}

void UVFX_Manager::SpawnDinosaurFootstep(FVector Location, float DinosaurSize)
{
    FVector Scale = FVector(DinosaurSize, DinosaurSize, DinosaurSize);
    SpawnEffect(EVFX_EffectType::DustCloud, Location, FRotator::ZeroRotator, Scale);
    
    // Add ground impact shake effect
    if (GetWorld())
    {
        FVector ImpactLocation = Location;
        ImpactLocation.Z -= 50.0f; // Ground level
        SpawnEffect(EVFX_EffectType::DustCloud, ImpactLocation, FRotator::ZeroRotator, Scale * 0.5f);
    }
}

void UVFX_Manager::SpawnBloodSplatter(FVector Location, FVector Direction, float Intensity)
{
    FRotator BloodRotation = Direction.Rotation();
    FVector BloodScale = FVector(Intensity, Intensity, Intensity);
    SpawnEffect(EVFX_EffectType::BloodSplatter, Location, BloodRotation, BloodScale);
}

void UVFX_Manager::SpawnDustCloud(FVector Location, float Radius)
{
    float Scale = Radius / 100.0f; // Normalize to base scale
    SpawnEffect(EVFX_EffectType::DustCloud, Location, FRotator::ZeroRotator, FVector(Scale));
}

void UVFX_Manager::SpawnWaterSplash(FVector Location, float Force)
{
    FVector SplashScale = FVector(Force, Force, Force);
    SpawnEffect(EVFX_EffectType::WaterSplash, Location, FRotator::ZeroRotator, SplashScale);
}

void UVFX_Manager::SpawnCampfireSmoke(FVector Location)
{
    FVector SmokeLocation = Location;
    SmokeLocation.Z += 100.0f; // Smoke rises
    SpawnEffect(EVFX_EffectType::CampfireSmoke, SmokeLocation, FRotator::ZeroRotator, FVector::OneVector);
}

void UVFX_Manager::CleanupFinishedEffects()
{
    for (int32 i = ActiveEffects.Num() - 1; i >= 0; --i)
    {
        UParticleSystemComponent* Effect = ActiveEffects[i];
        if (!Effect || !Effect->IsActive() || !IsValid(Effect))
        {
            ActiveEffects.RemoveAt(i);
        }
    }
}

void UVFX_Manager::StopAllEffects()
{
    for (UParticleSystemComponent* Effect : ActiveEffects)
    {
        if (Effect && IsValid(Effect))
        {
            Effect->DeactivateSystem();
        }
    }
    ActiveEffects.Empty();
    UE_LOG(LogTemp, Log, TEXT("All VFX effects stopped"));
}