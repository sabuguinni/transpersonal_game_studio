// DinosaurBase.cpp
// Transpersonal Game Studio — Dinosaur AI Base Implementation
// Agent #04 Performance Optimizer — completing pair from Agent #03

#include "DinosaurBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // 10Hz tick for performance

    // Capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(88.0f);
    GetCapsuleComponent()->SetCapsuleRadius(34.0f);

    // Skeletal mesh
    GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f));
    GetMesh()->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

    // Defaults
    DinosaurSpecies = EPerf_DinosaurSpecies::TRex;
    CurrentBehaviorState = EPerf_DinoState::Idle;
    MaxHealth = 1000.0f;
    CurrentHealth = 1000.0f;
    AttackDamage = 150.0f;
    DetectionRadius = 2000.0f;
    AttackRadius = 300.0f;
    AggressionLevel = 0.5f;
    bIsAggressive = false;
    bIsPredator = true;
    CurrentBiomeDangerLevel = 0.0f;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    UpdateAggressionFromBiome();
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Behavior tick at reduced rate for performance
    BehaviorTick(DeltaTime);
}

void ADinosaurBase::BehaviorTick(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
    case EPerf_DinoState::Idle:
        // Scan for player/prey periodically
        break;
    case EPerf_DinoState::Patrol:
        break;
    case EPerf_DinoState::Chase:
        break;
    case EPerf_DinoState::Attack:
        break;
    case EPerf_DinoState::Flee:
        break;
    case EPerf_DinoState::Dead:
        break;
    default:
        break;
    }
}

void ADinosaurBase::SetBiomeDangerLevel(float DangerLevel)
{
    CurrentBiomeDangerLevel = FMath::Clamp(DangerLevel, 0.0f, 1.0f);
    UpdateAggressionFromBiome();
}

void ADinosaurBase::UpdateAggressionFromBiome()
{
    // Higher biome danger = more aggressive dinosaur
    AggressionLevel = FMath::Clamp(0.3f + CurrentBiomeDangerLevel * 0.7f, 0.0f, 1.0f);
    bIsAggressive = (AggressionLevel > 0.6f);

    // Adjust detection radius based on aggression
    DetectionRadius = FMath::Lerp(1000.0f, 3000.0f, AggressionLevel);
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    if (CurrentHealth <= 0.0f && CurrentBehaviorState != EPerf_DinoState::Dead)
    {
        CurrentBehaviorState = EPerf_DinoState::Dead;
        OnDinosaurDied();
    }
    else if (bIsPredator && CurrentBehaviorState == EPerf_DinoState::Idle)
    {
        // Predators retaliate when hit
        CurrentBehaviorState = EPerf_DinoState::Chase;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDinosaurDied()
{
    // Disable collision, stop movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->StopMovementImmediately();
    GetCharacterMovement()->DisableMovement();

    // Ragdoll
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
    GetMesh()->SetSimulatePhysics(true);

    // Destroy after delay (cleanup for performance)
    SetLifeSpan(30.0f);
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && CurrentBehaviorState != EPerf_DinoState::Dead;
}
