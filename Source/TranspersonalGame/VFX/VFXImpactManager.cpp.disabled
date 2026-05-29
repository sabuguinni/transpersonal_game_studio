#include "VFXImpactManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

UVFX_ImpactManager::UVFX_ImpactManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default values
    MaxActiveParticles = 50;
    ParticleLifetime = 5.0f;
    bEnableVFXLOD = true;
    
    // Initialize particle system pointers to null
    FootstepDustSystem = nullptr;
    BloodSplatterSystem = nullptr;
    DustCloudSystem = nullptr;
    RockDebrisSystem = nullptr;
}

void UVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Try to load default particle systems from engine content
    // These are fallback systems in case custom ones aren't set
    if (!FootstepDustSystem)
    {
        FootstepDustSystem = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/VFX/P_Explosion.P_Explosion"));
    }
    
    if (!DustCloudSystem)
    {
        DustCloudSystem = LoadObject<UParticleSystem>(nullptr, TEXT("/Engine/VFX/P_Smoke.P_Smoke"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("VFX Impact Manager initialized with %d max particles"), MaxActiveParticles);
}

void UVFX_ImpactManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up finished particle components
    for (int32 i = ActiveParticleComponents.Num() - 1; i >= 0; i--)
    {
        if (ActiveParticleComponents[i] && !ActiveParticleComponents[i]->IsActive())
        {
            ActiveParticleComponents[i]->DestroyComponent();
            ActiveParticleComponents.RemoveAt(i);
        }
    }
    
    // Enforce maximum particle limit for performance
    while (ActiveParticleComponents.Num() > MaxActiveParticles)
    {
        if (ActiveParticleComponents[0])
        {
            ActiveParticleComponents[0]->DeactivateSystem();
            ActiveParticleComponents[0]->DestroyComponent();
        }
        ActiveParticleComponents.RemoveAt(0);
    }
}

void UVFX_ImpactManager::CreateImpactEffect(const FVFX_ImpactData& ImpactData)
{
    switch (ImpactData.ImpactType)
    {
        case EVFX_ImpactType::DinosaurFootstep:
            CreateDinosaurFootstepEffect(ImpactData.ImpactLocation, ImpactData.ParticleScale);
            break;
            
        case EVFX_ImpactType::BloodSplatter:
            CreateBloodSplatterEffect(ImpactData.ImpactLocation, ImpactData.ImpactNormal);
            break;
            
        case EVFX_ImpactType::DustCloud:
            CreateDustCloudEffect(ImpactData.ImpactLocation, ImpactData.ImpactForce);
            break;
            
        case EVFX_ImpactType::RockImpact:
            if (RockDebrisSystem)
            {
                SpawnParticleEffect(RockDebrisSystem, ImpactData.ImpactLocation, FRotator::ZeroRotator, ImpactData.ParticleScale);
            }
            break;
            
        default:
            CreateDustCloudEffect(ImpactData.ImpactLocation, ImpactData.ImpactForce);
            break;
    }
}

void UVFX_ImpactManager::CreateDinosaurFootstepEffect(FVector Location, float DinosaurSize)
{
    if (!FootstepDustSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("FootstepDustSystem not set, using fallback"));
        return;
    }
    
    // Scale effect based on dinosaur size
    float EffectScale = FMath::Clamp(DinosaurSize, 0.5f, 3.0f);
    
    // Spawn dust cloud at ground level
    FVector GroundLocation = Location;
    GroundLocation.Z -= 50.0f; // Offset to ground
    
    SpawnParticleEffect(FootstepDustSystem, GroundLocation, FRotator::ZeroRotator, EffectScale);
    
    // Add screen shake for large dinosaurs
    if (DinosaurSize > 2.0f)
    {
        UGameplayStatics::PlayWorldCameraShake(
            GetWorld(),
            nullptr, // Use default camera shake class
            Location,
            100.0f,  // Inner radius
            1000.0f, // Outer radius
            1.0f,    // Falloff
            false    // Orient shake towards epicenter
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Dinosaur footstep VFX created at %s with scale %f"), *Location.ToString(), EffectScale);
}

void UVFX_ImpactManager::CreateBloodSplatterEffect(FVector Location, FVector Direction)
{
    if (!BloodSplatterSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("BloodSplatterSystem not set"));
        return;
    }
    
    // Calculate rotation from direction vector
    FRotator EffectRotation = Direction.Rotation();
    
    SpawnParticleEffect(BloodSplatterSystem, Location, EffectRotation, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Blood splatter VFX created at %s"), *Location.ToString());
}

void UVFX_ImpactManager::CreateDustCloudEffect(FVector Location, float Intensity)
{
    if (!DustCloudSystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("DustCloudSystem not set"));
        return;
    }
    
    float EffectScale = FMath::Clamp(Intensity, 0.1f, 2.0f);
    
    SpawnParticleEffect(DustCloudSystem, Location, FRotator::ZeroRotator, EffectScale);
    
    UE_LOG(LogTemp, Log, TEXT("Dust cloud VFX created at %s with intensity %f"), *Location.ToString(), Intensity);
}

void UVFX_ImpactManager::SpawnParticleEffect(UParticleSystem* ParticleSystem, const FVector& Location, const FRotator& Rotation, float Scale)
{
    if (!ParticleSystem || !GetWorld())
    {
        return;
    }
    
    // Use UGameplayStatics for one-shot effects
    UParticleSystemComponent* ParticleComp = UGameplayStatics::SpawnEmitterAtLocation(
        GetWorld(),
        ParticleSystem,
        Location,
        Rotation,
        FVector(Scale),
        true, // Auto destroy
        EPSCPoolMethod::None,
        true  // Auto activate
    );
    
    if (ParticleComp)
    {
        // Add to active list for tracking
        ActiveParticleComponents.Add(ParticleComp);
        
        // Set lifetime
        ParticleComp->SetFloatParameter(FName("Lifetime"), ParticleLifetime);
        
        UE_LOG(LogTemp, Log, TEXT("Particle effect spawned at %s"), *Location.ToString());
    }
}