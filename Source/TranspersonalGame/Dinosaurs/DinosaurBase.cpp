// DinosaurBase.cpp — Engine Architect #02 — PROD_CYCLE_AUTO_20260702_002
// Base class for all dinosaur species. Implements survival AI hooks,
// movement configuration, and damage response for the prehistoric world.

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default species data
    SpeciesData.SpeciesName = FName("UnknownDinosaur");
    SpeciesData.MaxHealth = 100.0f;
    SpeciesData.MoveSpeed = 400.0f;
    SpeciesData.AttackDamage = 20.0f;
    SpeciesData.DetectionRadius = 1500.0f;
    SpeciesData.bIsPredator = false;
    SpeciesData.DietType = EDinoSpeciesDiet::Herbivore;

    // Runtime state
    CurrentHealth = SpeciesData.MaxHealth;
    CurrentState = EDinosaurState::Idle;
    bIsAlerted = false;
    bIsDead = false;
    AlertCooldown = 5.0f;

    // Configure capsule
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(88.0f);
        Capsule->SetCapsuleRadius(34.0f);
    }

    // Configure movement for terrain traversal
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = SpeciesData.MoveSpeed;
        MoveComp->MaxStepHeight = 60.0f;
        MoveComp->WalkableFloorAngle = 50.0f;
        MoveComp->GravityScale = 1.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    }

    // No controller auto-possession — AI controller handles this
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialize health from species data
    CurrentHealth = SpeciesData.MaxHealth;
    CurrentState = EDinosaurState::Idle;

    // Apply movement speed from species data
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = SpeciesData.MoveSpeed;
    }

    // Start idle behavior tick
    GetWorldTimerManager().SetTimer(
        BehaviorTickHandle,
        this,
        &ADinosaurBase::UpdateBehaviorState,
        2.0f,   // Every 2 seconds
        true    // Looping
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead)
    {
        return;
    }

    // Tick current behavior
    switch (CurrentState)
    {
        case EDinosaurState::Idle:
            OnIdle(DeltaTime);
            break;
        case EDinosaurState::Roaming:
            OnRoam(DeltaTime);
            break;
        case EDinosaurState::Alerted:
            OnAlert(DeltaTime);
            break;
        case EDinosaurState::Attacking:
            OnAttack(DeltaTime);
            break;
        case EDinosaurState::Fleeing:
            OnFlee(DeltaTime);
            break;
        case EDinosaurState::Dead:
            // Handled by death logic
            break;
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (bIsDead)
    {
        return 0.0f;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, SpeciesData.MaxHealth);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else
    {
        // Alert on damage — become aggressive if predator
        if (SpeciesData.bIsPredator && CurrentState != EDinosaurState::Attacking)
        {
            SetState(EDinosaurState::Alerted);
            bIsAlerted = true;
        }
        OnDamageReceived(ActualDamage, DamageCauser);
    }

    return ActualDamage;
}

void ADinosaurBase::SetState(EDinosaurState NewState)
{
    if (CurrentState == NewState || bIsDead)
    {
        return;
    }

    EDinosaurState OldState = CurrentState;
    CurrentState = NewState;
    OnStateChanged(OldState, NewState);
}

void ADinosaurBase::AlertDinosaur(AActor* Threat)
{
    if (bIsDead || !Threat)
    {
        return;
    }

    bIsAlerted = true;
    AlertTarget = Threat;

    if (SpeciesData.bIsPredator)
    {
        SetState(EDinosaurState::Attacking);
    }
    else
    {
        SetState(EDinosaurState::Fleeing);
    }
}

void ADinosaurBase::Die()
{
    if (bIsDead)
    {
        return;
    }

    bIsDead = true;
    CurrentHealth = 0.0f;
    SetState(EDinosaurState::Dead);

    // Stop behavior timer
    GetWorldTimerManager().ClearTimer(BehaviorTickHandle);

    // Disable collision
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Disable movement
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->DisableMovement();
    }

    OnDeath();
}

void ADinosaurBase::UpdateBehaviorState()
{
    if (bIsDead)
    {
        return;
    }

    // Simple state machine — subclasses override for species-specific behavior
    if (CurrentState == EDinosaurState::Idle)
    {
        // Randomly transition to roaming
        if (FMath::RandRange(0, 3) == 0)
        {
            SetState(EDinosaurState::Roaming);
        }
    }
    else if (CurrentState == EDinosaurState::Roaming)
    {
        // Occasionally rest
        if (FMath::RandRange(0, 5) == 0)
        {
            SetState(EDinosaurState::Idle);
        }
    }
}

// Default behavior implementations — subclasses override
void ADinosaurBase::OnIdle(float DeltaTime) {}
void ADinosaurBase::OnRoam(float DeltaTime) {}
void ADinosaurBase::OnAlert(float DeltaTime) {}
void ADinosaurBase::OnAttack(float DeltaTime) {}
void ADinosaurBase::OnFlee(float DeltaTime) {}
void ADinosaurBase::OnDeath() {}
void ADinosaurBase::OnDamageReceived(float Damage, AActor* DamageCauser) {}
void ADinosaurBase::OnStateChanged(EDinosaurState OldState, EDinosaurState NewState) {}
