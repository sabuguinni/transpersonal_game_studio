// DinosaurBase.cpp
// Engine Architect #02 — Cycle PROD_CYCLE_AUTO_20260620_007
// Base class for all dinosaur pawns in the prehistoric survival game.
// All dinosaur types (TRex, Raptor, Brachiosaurus, etc.) inherit from this.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species in child classes
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 0.0f;  // Dinosaurs don't jump by default
    GetCharacterMovement()->GravityScale = 1.0f;

    // Species defaults
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MoveSpeed = 400.0f;
    AttackDamage = 50.0f;
    DetectionRadius = 1500.0f;
    AttackRadius = 200.0f;
    bIsAggressive = false;
    bIsPack = false;
    SpeciesName = TEXT("Unknown");
    DinoState = EDinoState::Idle;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Apply species-specific movement speed
    GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // AI tick handled by BehaviorTree / AIController in child classes
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }
    else if (bIsAggressive && DinoState == EDinoState::Idle)
    {
        // Provoked — switch to aggressive state
        DinoState = EDinoState::Aggressive;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    DinoState = EDinoState::Dead;

    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    // Broadcast death event for quest/loot systems
    OnDinosaurDied.Broadcast(this);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f && DinoState != EDinoState::Dead;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ADinosaurBase::SetDinoState(EDinoState NewState)
{
    if (DinoState != EDinoState::Dead)
    {
        DinoState = NewState;
    }
}
