#include "VFX_FootstepImpactSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/HitResult.h"

UVFX_FootstepImpactSystem::UVFX_FootstepImpactSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
    
    MaxImpactDistance = 2000.0f;
    bEnableDustClouds = true;
    bEnableDebrisEffects = true;
    bEnableGroundCracks = true;
    bEnableFootstepSounds = true;
}

void UVFX_FootstepImpactSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeParticleSystems();
    InitializeAudioSystems();
}

void UVFX_FootstepImpactSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UVFX_FootstepImpactSystem::TriggerFootstepImpact(const FVector& Location, EVFX_FootstepType FootstepType, const FVFX_FootstepData& FootstepData)
{
    if (!GetWorld())
    {
        return;
    }

    // Calculate impact intensity based on footstep type and force
    float ImpactIntensity = CalculateImpactIntensity(FootstepType, FootstepData.ImpactForce);
    
    // Get surface type at impact location
    ESurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    
    // Create dust cloud effect
    if (bEnableDustClouds && ImpactIntensity > 0.1f)
    {
        CreateDustCloud(Location, FootstepData.DustCloudScale * ImpactIntensity, SurfaceType);
    }
    
    // Create debris effect for heavy impacts
    if (bEnableDebrisEffects && ImpactIntensity > 0.5f)
    {
        CreateDebrisEffect(Location, FootstepData.DebrisDirection, FootstepData.ImpactForce);
    }
    
    // Create ground cracks for very heavy impacts
    if (bEnableGroundCracks && FootstepData.bCreateGroundCracks && ImpactIntensity > 0.8f)
    {
        CreateGroundCracks(Location, ImpactIntensity * 100.0f);
    }
    
    // Play footstep sound
    if (bEnableFootstepSounds)
    {
        PlayFootstepSound(Location, FootstepType, SurfaceType);
    }
}

void UVFX_FootstepImpactSystem::CreateDustCloud(const FVector& Location, float Scale, ESurfaceType Surface)
{
    if (!GetWorld())
    {
        return;
    }

    UParticleSystem* DustParticle = nullptr;
    if (DustCloudParticles.Contains(Surface))
    {
        DustParticle = DustCloudParticles[Surface];
    }
    
    if (DustParticle)
    {
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DustParticle,
            Location,
            FRotator::ZeroRotator,
            FVector(Scale, Scale, Scale)
        );
    }
}

void UVFX_FootstepImpactSystem::CreateDebrisEffect(const FVector& Location, const FVector& Direction, float Force)
{
    if (!GetWorld())
    {
        return;
    }

    // Create debris particles with physics simulation
    ESurfaceType SurfaceType = GetSurfaceTypeAtLocation(Location);
    UParticleSystem* DebrisParticle = nullptr;
    
    if (DebrisParticles.Contains(SurfaceType))
    {
        DebrisParticle = DebrisParticles[SurfaceType];
    }
    
    if (DebrisParticle)
    {
        FVector DebrisDirection = Direction.IsNearlyZero() ? FVector::UpVector : Direction.GetSafeNormal();
        FRotator DebrisRotation = DebrisDirection.Rotation();
        
        UGameplayStatics::SpawnEmitterAtLocation(
            GetWorld(),
            DebrisParticle,
            Location,
            DebrisRotation,
            FVector(Force * 0.1f, Force * 0.1f, Force * 0.1f)
        );
    }
}

void UVFX_FootstepImpactSystem::CreateGroundCracks(const FVector& Location, float CrackRadius)
{
    if (!GetWorld())
    {
        return;
    }

    // Create ground crack decal effect
    // This would typically use a decal component or material effect
    // For now, we'll log the crack creation
    UE_LOG(LogTemp, Log, TEXT("VFX: Creating ground cracks at %s with radius %f"), *Location.ToString(), CrackRadius);
}

void UVFX_FootstepImpactSystem::PlayFootstepSound(const FVector& Location, EVFX_FootstepType FootstepType, ESurfaceType Surface)
{
    if (!GetWorld())
    {
        return;
    }

    USoundCue* FootstepSound = nullptr;
    if (FootstepSounds.Contains(FootstepType))
    {
        FootstepSound = FootstepSounds[FootstepType];
    }
    
    if (FootstepSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            FootstepSound,
            Location,
            1.0f,  // Volume
            1.0f,  // Pitch
            0.0f   // Start time
        );
    }
}

void UVFX_FootstepImpactSystem::InitializeParticleSystems()
{
    // Initialize particle system maps with default values
    // These would be loaded from content assets in a real implementation
    DustCloudParticles.Empty();
    DebrisParticles.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX FootstepImpactSystem: Particle systems initialized"));
}

void UVFX_FootstepImpactSystem::InitializeAudioSystems()
{
    // Initialize audio system maps with default values
    // These would be loaded from content assets in a real implementation
    FootstepSounds.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VFX FootstepImpactSystem: Audio systems initialized"));
}

ESurfaceType UVFX_FootstepImpactSystem::GetSurfaceTypeAtLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return ESurfaceType::Default;
    }

    // Perform line trace to get surface material
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.PhysMaterial.IsValid())
        {
            return HitResult.PhysMaterial->SurfaceType;
        }
    }
    
    return ESurfaceType::Default;
}

float UVFX_FootstepImpactSystem::CalculateImpactIntensity(EVFX_FootstepType FootstepType, float ImpactForce)
{
    float BaseIntensity = 0.5f;
    
    switch (FootstepType)
    {
        case EVFX_FootstepType::Light:
            BaseIntensity = 0.2f;
            break;
        case EVFX_FootstepType::Medium:
            BaseIntensity = 0.5f;
            break;
        case EVFX_FootstepType::Heavy:
            BaseIntensity = 0.8f;
            break;
        case EVFX_FootstepType::Dinosaur:
            BaseIntensity = 1.2f;
            break;
        case EVFX_FootstepType::Giant:
            BaseIntensity = 2.0f;
            break;
    }
    
    return FMath::Clamp(BaseIntensity * ImpactForce, 0.0f, 3.0f);
}