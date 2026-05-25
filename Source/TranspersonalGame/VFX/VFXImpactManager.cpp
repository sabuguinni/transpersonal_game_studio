#include "VFXImpactManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"

AVFX_ImpactManager::AVFX_ImpactManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create particle system components
    DustParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("DustParticleComponent"));
    DustParticleComponent->SetupAttachment(RootComponent);
    DustParticleComponent->bAutoActivate = false;

    BloodParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("BloodParticleComponent"));
    BloodParticleComponent->SetupAttachment(RootComponent);
    BloodParticleComponent->bAutoActivate = false;

    // Initialize default values
    MaxImpactDistance = 5000.0f;
    DustLifetime = 3.0f;
    BloodLifetime = 8.0f;
}

void AVFX_ImpactManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeParticleSystems();
}

void AVFX_ImpactManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateImpactEffects(DeltaTime);
    CleanupExpiredEffects();
}

void AVFX_ImpactManager::CreateFootstepImpact(const FVector& Location, float DinosaurMass)
{
    if (!DustParticleComponent)
    {
        return;
    }

    // Calculate dust cloud size based on dinosaur mass
    float CloudSize = FMath::Clamp(DinosaurMass * 0.1f, 50.0f, 500.0f);
    
    // Create impact data
    FVFX_ImpactData NewImpact;
    NewImpact.ImpactLocation = Location;
    NewImpact.ImpactForce = DinosaurMass;
    NewImpact.ImpactType = EVFX_ImpactType::Footstep;
    NewImpact.DustCloudSize = CloudSize;
    
    ActiveImpacts.Add(NewImpact);

    // Spawn dust cloud effect
    CreateDustCloud(Location, CloudSize);

    UE_LOG(LogTemp, Log, TEXT("VFX: Footstep impact created at %s with mass %f"), 
           *Location.ToString(), DinosaurMass);
}

void AVFX_ImpactManager::CreateBloodSplatter(const FVector& Location, const FVector& Direction, float Intensity)
{
    if (!BloodParticleComponent)
    {
        return;
    }

    // Create blood impact data
    FVFX_ImpactData BloodImpact;
    BloodImpact.ImpactLocation = Location;
    BloodImpact.ImpactForce = Intensity;
    BloodImpact.ImpactType = EVFX_ImpactType::Blood;
    BloodImpact.BloodColor = FLinearColor(0.8f, 0.1f, 0.1f, 1.0f); // Dark red

    ActiveImpacts.Add(BloodImpact);

    // Set particle system location and activate
    BloodParticleComponent->SetWorldLocation(Location);
    BloodParticleComponent->SetVectorParameter(TEXT("ImpactDirection"), Direction);
    BloodParticleComponent->SetFloatParameter(TEXT("Intensity"), Intensity);
    BloodParticleComponent->ActivateSystem();

    UE_LOG(LogTemp, Log, TEXT("VFX: Blood splatter created at %s with intensity %f"), 
           *Location.ToString(), Intensity);
}

void AVFX_ImpactManager::CreateDustCloud(const FVector& Location, float CloudSize)
{
    if (!DustParticleComponent)
    {
        return;
    }

    // Set dust particle parameters
    DustParticleComponent->SetWorldLocation(Location);
    DustParticleComponent->SetFloatParameter(TEXT("CloudSize"), CloudSize);
    DustParticleComponent->SetFloatParameter(TEXT("Lifetime"), DustLifetime);
    
    // Activate dust system
    DustParticleComponent->ActivateSystem();

    UE_LOG(LogTemp, Log, TEXT("VFX: Dust cloud created at %s with size %f"), 
           *Location.ToString(), CloudSize);
}

void AVFX_ImpactManager::CreateCombatImpact(const FVector& Location, EVFX_ImpactType Type, float Force)
{
    FVFX_ImpactData CombatImpact;
    CombatImpact.ImpactLocation = Location;
    CombatImpact.ImpactForce = Force;
    CombatImpact.ImpactType = Type;

    switch (Type)
    {
        case EVFX_ImpactType::Bite:
            CreateBloodSplatter(Location, FVector::UpVector, Force * 0.5f);
            break;
        case EVFX_ImpactType::Claw:
            CreateBloodSplatter(Location, FVector::ForwardVector, Force * 0.3f);
            CreateDustCloud(Location, Force * 0.2f);
            break;
        case EVFX_ImpactType::Tail:
            CreateDustCloud(Location, Force * 0.4f);
            break;
        default:
            CreateDustCloud(Location, Force * 0.1f);
            break;
    }

    ActiveImpacts.Add(CombatImpact);
}

void AVFX_ImpactManager::UpdateVFXLOD(float DistanceToPlayer)
{
    if (!DustParticleComponent || !BloodParticleComponent)
    {
        return;
    }

    // Calculate LOD level based on distance
    float LODLevel = FMath::Clamp(DistanceToPlayer / 1000.0f, 0.1f, 1.0f);
    
    // Adjust particle density based on distance
    DustParticleComponent->SetFloatParameter(TEXT("LODLevel"), LODLevel);
    BloodParticleComponent->SetFloatParameter(TEXT("LODLevel"), LODLevel);

    // Disable effects if too far away
    if (DistanceToPlayer > MaxImpactDistance)
    {
        DustParticleComponent->DeactivateSystem();
        BloodParticleComponent->DeactivateSystem();
    }
}

void AVFX_ImpactManager::CleanupExpiredEffects()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    ActiveImpacts.RemoveAll([CurrentTime, this](const FVFX_ImpactData& Impact)
    {
        float EffectLifetime = (Impact.ImpactType == EVFX_ImpactType::Blood) ? BloodLifetime : DustLifetime;
        return (CurrentTime - Impact.ImpactForce) > EffectLifetime;
    });
}

void AVFX_ImpactManager::TestFootstepVFX()
{
    FVector TestLocation = GetActorLocation() + FVector(0, 0, -100);
    CreateFootstepImpact(TestLocation, 5000.0f); // T-Rex mass
    UE_LOG(LogTemp, Warning, TEXT("VFX Test: Footstep effect triggered at %s"), *TestLocation.ToString());
}

void AVFX_ImpactManager::TestBloodVFX()
{
    FVector TestLocation = GetActorLocation();
    CreateBloodSplatter(TestLocation, FVector::UpVector, 100.0f);
    UE_LOG(LogTemp, Warning, TEXT("VFX Test: Blood effect triggered at %s"), *TestLocation.ToString());
}

void AVFX_ImpactManager::InitializeParticleSystems()
{
    // Initialize particle system parameters
    if (DustParticleComponent)
    {
        DustParticleComponent->SetFloatParameter(TEXT("DefaultSize"), 100.0f);
        DustParticleComponent->SetVectorParameter(TEXT("DefaultColor"), FVector(0.6f, 0.4f, 0.2f));
    }

    if (BloodParticleComponent)
    {
        BloodParticleComponent->SetFloatParameter(TEXT("DefaultIntensity"), 50.0f);
        BloodParticleComponent->SetVectorParameter(TEXT("BloodColor"), FVector(0.8f, 0.1f, 0.1f));
    }

    UE_LOG(LogTemp, Log, TEXT("VFX Impact Manager: Particle systems initialized"));
}

void AVFX_ImpactManager::UpdateImpactEffects(float DeltaTime)
{
    // Update active impact effects
    for (FVFX_ImpactData& Impact : ActiveImpacts)
    {
        // Fade out effects over time
        Impact.ImpactForce *= 0.98f; // Gradual fade
        
        if (Impact.ImpactForce < 1.0f)
        {
            Impact.ImpactForce = 0.0f; // Mark for cleanup
        }
    }
}