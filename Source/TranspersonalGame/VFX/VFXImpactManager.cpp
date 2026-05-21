#include "VFXImpactManager.h"
#include "Engine/Engine.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SceneComponent.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
    RootComponent = RootSceneComponent;

    // Create particle system components
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticles"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = false;

    BloodParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BloodParticles"));
    BloodParticleComponent->SetupAttachment(RootComponent);
    BloodParticleComponent->bAutoActivate = false;

    // Initialize VFX settings
    FootstepDustIntensity = 1.0f;
    BloodSplatterScale = 1.0f;
    EnvironmentalDustRadius = 200.0f;

    // Initialize footstep scale map
    FootstepScaleMap.Add(EDinosaurSpecies::TRex, 3.0f);
    FootstepScaleMap.Add(EDinosaurSpecies::Velociraptor, 0.8f);
    FootstepScaleMap.Add(EDinosaurSpecies::Triceratops, 2.5f);
    FootstepScaleMap.Add(EDinosaurSpecies::Brachiosaurus, 4.0f);
    FootstepScaleMap.Add(EDinosaurSpecies::Ankylosaurus, 2.0f);
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeParticleSystems();
}

void AVFX_ImpactManager::InitializeParticleSystems()
{
    // Load default particle systems if available
    if (DustParticleComponent)
    {
        DustParticleComponent->SetWorldScale3D(FVector(FootstepDustIntensity));
    }

    if (BloodParticleComponent)
    {
        BloodParticleComponent->SetWorldScale3D(FVector(BloodSplatterScale));
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager initialized with particle systems"));
}

void AVFX_ImpactManager::TriggerFootstepImpact(const FVFX_ImpactData& ImpactData)
{
    if (!DustParticleComponent)
    {
        return;
    }

    // Configure VFX based on dinosaur species
    ConfigureFootstepVFX(ImpactData.DinosaurType);

    // Set particle system location and rotation
    FVector ParticleLocation = ImpactData.ImpactLocation;
    FRotator ParticleRotation = FRotationMatrix::MakeFromZ(ImpactData.ImpactNormal).Rotator();

    DustParticleComponent->SetWorldLocationAndRotation(ParticleLocation, ParticleRotation);

    // Scale based on impact force and dinosaur size
    float* SpeciesScale = FootstepScaleMap.Find(ImpactData.DinosaurType);
    float FinalScale = SpeciesScale ? (*SpeciesScale * ImpactData.ImpactForce) : ImpactData.ImpactForce;
    
    DustParticleComponent->SetWorldScale3D(FVector(FinalScale));

    // Activate particle system
    DustParticleComponent->ActivateSystem();

    UE_LOG(LogTemp, Log, TEXT("Footstep impact VFX triggered at location: %s"), 
           *ImpactData.ImpactLocation.ToString());
}

void AVFX_ImpactManager::TriggerBloodSplatter(const FVector& Location, float Intensity)
{
    if (!BloodParticleComponent)
    {
        return;
    }

    BloodParticleComponent->SetWorldLocation(Location);
    BloodParticleComponent->SetWorldScale3D(FVector(Intensity * BloodSplatterScale));
    BloodParticleComponent->ActivateSystem();

    UE_LOG(LogTemp, Log, TEXT("Blood splatter VFX triggered at location: %s"), *Location.ToString());
}

void AVFX_ImpactManager::TriggerEnvironmentalDust(const FVector& Location, float Radius)
{
    if (!DustParticleComponent)
    {
        return;
    }

    DustParticleComponent->SetWorldLocation(Location);
    float Scale = Radius / EnvironmentalDustRadius;
    DustParticleComponent->SetWorldScale3D(FVector(Scale));
    DustParticleComponent->ActivateSystem();

    UE_LOG(LogTemp, Log, TEXT("Environmental dust VFX triggered at location: %s with radius: %f"), 
           *Location.ToString(), Radius);
}

void AVFX_ImpactManager::SetDinosaurFootstepScale(EDinosaurSpecies Species, float Scale)
{
    FootstepScaleMap.Add(Species, Scale);
    UE_LOG(LogTemp, Log, TEXT("Footstep scale set for species %d: %f"), 
           static_cast<int32>(Species), Scale);
}

void AVFX_ImpactManager::ConfigureFootstepVFX(EDinosaurSpecies Species)
{
    // Configure particle parameters based on dinosaur species
    switch (Species)
    {
        case EDinosaurSpecies::TRex:
            FootstepDustIntensity = 3.0f;
            break;
        case EDinosaurSpecies::Velociraptor:
            FootstepDustIntensity = 0.8f;
            break;
        case EDinosaurSpecies::Triceratops:
            FootstepDustIntensity = 2.5f;
            break;
        case EDinosaurSpecies::Brachiosaurus:
            FootstepDustIntensity = 4.0f;
            break;
        case EDinosaurSpecies::Ankylosaurus:
            FootstepDustIntensity = 2.0f;
            break;
        default:
            FootstepDustIntensity = 1.0f;
            break;
    }
}