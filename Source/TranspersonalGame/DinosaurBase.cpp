// DinosaurBase.cpp — Base class implementation for all dinosaur pawns
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260626_012

#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default stats — overridden per species subclass
    MaxHealth       = 500.0f;
    CurrentHealth   = 500.0f;
    MaxStamina      = 300.0f;
    CurrentStamina  = 300.0f;
    HungerLevel     = 0.0f;
    ThirstLevel     = 0.0f;

    AttackDamage    = 40.0f;
    AttackRange     = 200.0f;
    DetectionRadius = 1500.0f;
    bIsAggressive   = false;
    bIsAlerted      = false;

    MovementSpeed   = 400.0f;
    SprintSpeed     = 700.0f;

    DinosaurSpecies = EDinosaurSpecies::Unknown;
    BehaviorState   = EDinosaurBehavior::Idle;

    // Configure movement component
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed        = MovementSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate        = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale        = 1.0f;
        MoveComp->MaxAcceleration     = 1024.0f;
        MoveComp->BrakingDecelerationWalking = 512.0f;
    }

    // Capsule defaults
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCapsuleHalfHeight(88.0f);
        Capsule->SetCapsuleRadius(34.0f);
    }

    // Disable controller rotation — movement component handles orientation
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw   = false;
    bUseControllerRotationRoll  = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Initialise stats to full
    CurrentHealth  = MaxHealth;
    CurrentStamina = MaxStamina;

    // Apply movement speed from property
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->MaxWalkSpeed = MovementSpeed;
    }
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive hunger/thirst drain (very slow — 1 unit per 10 seconds)
    if (IsAlive())
    {
        HungerLevel = FMath::Clamp(HungerLevel + DeltaTime * 0.1f, 0.0f, 100.0f);
        ThirstLevel = FMath::Clamp(ThirstLevel + DeltaTime * 0.08f, 0.0f, 100.0f);

        // Stamina regeneration when not sprinting
        CurrentStamina = FMath::Clamp(CurrentStamina + DeltaTime * 20.0f, 0.0f, MaxStamina);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.0f && IsAlive())
    {
        CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

        // Alert the dinosaur when hit
        bIsAlerted = true;

        if (!IsAlive())
        {
            OnDeath();
        }
    }

    return ActualDamage;
}

// --- Public API ---

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
    if (BehaviorState != NewState)
    {
        BehaviorState = NewState;
        OnBehaviorStateChanged(NewState);
    }
}

// --- Protected ---

void ADinosaurBase::OnDeath()
{
    // Stop movement
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->DisableMovement();
        MoveComp->StopMovementImmediately();
    }

    BehaviorState = EDinosaurBehavior::Dead;

    // Disable collision so the player can walk through the corpse
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Fire Blueprint event for death FX / ragdoll
    OnDeathBP();

    // Destroy actor after 10 seconds (corpse despawn)
    SetLifeSpan(10.0f);
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehavior NewState)
{
    // Adjust movement speed based on behavior
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        switch (NewState)
        {
        case EDinosaurBehavior::Hunting:
        case EDinosaurBehavior::Attacking:
            MoveComp->MaxWalkSpeed = SprintSpeed;
            break;
        case EDinosaurBehavior::Fleeing:
            MoveComp->MaxWalkSpeed = SprintSpeed * 1.2f;
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
    // Default: nothing — Blueprint subclasses override for death FX
}
