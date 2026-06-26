// DinosaurBase.cpp — Base class implementation for all dinosaur pawns
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260626_010

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default stats — overridden per species in subclasses
    MaxHealth       = 500.0f;
    CurrentHealth   = 500.0f;
    MaxStamina      = 300.0f;
    CurrentStamina  = 300.0f;
    HungerLevel     = 0.0f;
    ThirstLevel     = 0.0f;

    AttackDamage    = 50.0f;
    AttackRange     = 200.0f;
    DetectionRadius = 1500.0f;
    bIsAggressive   = false;
    bIsAlerted      = false;

    MovementSpeed   = 400.0f;
    SprintSpeed     = 800.0f;

    DinosaurSpecies = EDinosaurSpecies::Unknown;
    BehaviorState   = EDinosaurBehavior::Idle;

    // Configure movement component defaults
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed        = MovementSpeed;
        MoveComp->MaxAcceleration     = 1024.0f;
        MoveComp->BrakingDecelerationWalking = 512.0f;
        MoveComp->RotationRate        = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->bUseControllerDesiredRotation = false;
        MoveComp->GravityScale        = 1.0f;
        MoveComp->JumpZVelocity       = 0.0f; // Most dinos cannot jump
    }

    // Capsule defaults — subclasses resize per species
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCapsuleHalfHeight(88.0f);
        Capsule->SetCapsuleRadius(34.0f);
    }

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Ensure stats are fully initialised at runtime
    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;

    // Apply movement speed from property (may have been edited in Blueprint)
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = MovementSpeed;
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive hunger / thirst drain (very slow — background simulation)
    const float DrainRate = 0.5f; // units per second
    HungerLevel = FMath::Clamp(HungerLevel + DrainRate * DeltaTime, 0.0f, 100.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel + DrainRate * DeltaTime, 0.0f, 100.0f);

    // Stamina recovery when not sprinting
    if (BehaviorState != EDinosaurBehavior::Hunting && BehaviorState != EDinosaurBehavior::Attacking)
    {
        const float StaminaRecovery = 20.0f;
        CurrentStamina = FMath::Clamp(CurrentStamina + StaminaRecovery * DeltaTime, 0.0f, MaxStamina);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    if (ActualDamage <= 0.0f) return 0.0f;

    CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

    // Alert the dinosaur when hit
    if (!bIsAlerted)
    {
        bIsAlerted = true;
        SetBehaviorState(EDinosaurBehavior::Attacking);
    }

    if (CurrentHealth <= 0.0f)
    {
        OnDeath();
    }

    return ActualDamage;
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (BehaviorState == NewState) return;
    BehaviorState = NewState;
    OnBehaviorStateChanged(NewState);
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EDinosaurBehavior::Dead;

    // Disable movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
    }

    // Disable collision so the player can walk over the corpse
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Fire Blueprint event for death animation / ragdoll
    OnDeathBP();
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehavior NewState)
{
    // Update movement speed based on behavior
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EDinosaurBehavior::Hunting:
        case EDinosaurBehavior::Attacking:
            MoveComp->MaxWalkSpeed = SprintSpeed;
            break;
        case EDinosaurBehavior::Fleeing:
            MoveComp->MaxWalkSpeed = SprintSpeed * 0.85f;
            break;
        case EDinosaurBehavior::Resting:
        case EDinosaurBehavior::Dead:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        default:
            MoveComp->MaxWalkSpeed = MovementSpeed;
            break;
        }
    }
}

void ADinosaurBase::OnDeathBP_Implementation()
{
    // Default: nothing — Blueprint subclasses override for ragdoll / VFX
}
