// DinosaurBase.cpp — Base class for all dinosaur pawns
// Engine Architect #02 — Transpersonal Game Studio
// Cycle: PROD_CYCLE_AUTO_20260625_007

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default survival stats
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 200.0f;
    CurrentStamina = 200.0f;
    HungerLevel = 0.5f;
    ThirstLevel = 0.5f;
    bIsAggressive = false;
    bIsAlerted = false;
    DetectionRadius = 1500.0f;
    AttackRange = 300.0f;
    AttackDamage = 50.0f;
    MovementSpeed = 400.0f;
    SprintSpeed = 800.0f;
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    BehaviorState = EDinosaurBehavior::Idle;

    // Configure capsule
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 160.0f);

    // Configure movement
    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
    GetCharacterMovement()->MaxAcceleration = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;

    // No auto-possess — AI controller handles this
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    CurrentStamina = MaxStamina;
    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Passive hunger/thirst drain
    HungerLevel = FMath::Clamp(HungerLevel + DeltaTime * 0.001f, 0.0f, 1.0f);
    ThirstLevel = FMath::Clamp(ThirstLevel + DeltaTime * 0.0015f, 0.0f, 1.0f);

    // Stamina recovery when idle
    if (BehaviorState == EDinosaurBehavior::Idle || BehaviorState == EDinosaurBehavior::Resting)
    {
        CurrentStamina = FMath::Clamp(CurrentStamina + DeltaTime * 10.0f, 0.0f, MaxStamina);
    }
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
    else
    {
        // Alert on damage
        bIsAlerted = true;
        bIsAggressive = true;
        BehaviorState = EDinosaurBehavior::Attacking;
    }

    return ActualDamage;
}

void ADinosaurBase::OnDeath()
{
    BehaviorState = EDinosaurBehavior::Dead;
    // Disable collision and movement
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();
    // Notify blueprint
    OnDeathBP();
}

void ADinosaurBase::OnDeathBP_Implementation()
{
    // Default: ragdoll on death
    GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
}

bool ADinosaurBase::IsAlive() const
{
    return CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

void ADinosaurBase::SetBehaviorState(EDinosaurBehavior NewState)
{
    if (BehaviorState != NewState)
    {
        BehaviorState = NewState;
        OnBehaviorStateChanged(NewState);
    }
}

void ADinosaurBase::OnBehaviorStateChanged(EDinosaurBehavior NewState)
{
    switch (NewState)
    {
        case EDinosaurBehavior::Fleeing:
        case EDinosaurBehavior::Attacking:
            GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
            break;
        default:
            GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
            break;
    }
}
