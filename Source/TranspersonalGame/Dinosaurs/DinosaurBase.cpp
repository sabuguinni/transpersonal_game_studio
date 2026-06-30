// DinosaurBase.cpp
// Engine Architect #02 — PROD_CYCLE_AUTO_20260630_005
// Base class for all dinosaur pawns in the prehistoric survival game.
// Provides: health, stamina, territory radius, aggression, basic AI state.

#include "DinosaurBase.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Capsule defaults — overridden per species
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 120.0f);

    // Movement defaults
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = 400.0f;
    GetCharacterMovement()->JumpZVelocity = 500.0f;
    GetCharacterMovement()->AirControl = 0.2f;
    GetCharacterMovement()->GravityScale = 1.2f;

    // Survival stats defaults
    MaxHealth = 500.0f;
    CurrentHealth = 500.0f;
    MaxStamina = 200.0f;
    CurrentStamina = 200.0f;

    // Behaviour defaults
    TerritoryRadius = 2000.0f;
    AggressionLevel = 0.5f;
    DetectionRange = 1500.0f;
    AttackDamage = 40.0f;
    AttackCooldown = 2.0f;

    // AI state
    CurrentAIState = EDinoAIState::Idle;
    bIsAlive = true;
    bIsAggressive = false;

    // Species
    DinosaurSpecies = EDinosaurSpecies::Unknown;
    DinosaurSize = EDinosaurSize::Medium;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Store home location for territory logic
    HomeLocation = GetActorLocation();

    // Start idle behaviour timer
    GetWorldTimerManager().SetTimer(
        BehaviourTimerHandle,
        this,
        &ADinosaurBase::UpdateBehaviour,
        2.0f,
        true
    );
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Stamina regeneration
    if (CurrentStamina < MaxStamina)
    {
        CurrentStamina = FMath::Min(MaxStamina, CurrentStamina + StaminaRegenRate * DeltaTime);
    }
}

float ADinosaurBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (!bIsAlive) return 0.0f;

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

    OnDinosaurDamaged(ActualDamage, DamageCauser);

    if (CurrentHealth <= 0.0f)
    {
        Die();
    }
    else if (!bIsAggressive && DamageCauser)
    {
        // Become aggressive when attacked
        bIsAggressive = true;
        CurrentAIState = EDinoAIState::Attacking;
        TargetActor = DamageCauser;
    }

    return ActualDamage;
}

void ADinosaurBase::Die()
{
    if (!bIsAlive) return;

    bIsAlive = false;
    CurrentAIState = EDinoAIState::Dead;

    GetWorldTimerManager().ClearTimer(BehaviourTimerHandle);

    // Disable collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCharacterMovement()->DisableMovement();

    OnDinosaurDied();
}

void ADinosaurBase::UpdateBehaviour()
{
    if (!bIsAlive) return;

    switch (CurrentAIState)
    {
        case EDinoAIState::Idle:
            TickIdleBehaviour();
            break;
        case EDinoAIState::Patrolling:
            TickPatrolBehaviour();
            break;
        case EDinoAIState::Fleeing:
            TickFleeBehaviour();
            break;
        case EDinoAIState::Attacking:
            TickAttackBehaviour();
            break;
        default:
            break;
    }
}

void ADinosaurBase::TickIdleBehaviour()
{
    // Occasionally start patrolling
    float Rand = FMath::FRand();
    if (Rand < 0.3f)
    {
        CurrentAIState = EDinoAIState::Patrolling;
    }
}

void ADinosaurBase::TickPatrolBehaviour()
{
    // Return to idle if far from home
    float DistFromHome = FVector::Dist(GetActorLocation(), HomeLocation);
    if (DistFromHome > TerritoryRadius)
    {
        CurrentAIState = EDinoAIState::Idle;
    }
}

void ADinosaurBase::TickFleeBehaviour()
{
    // Stop fleeing after moving far enough
    if (TargetActor)
    {
        float DistFromThreat = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
        if (DistFromThreat > DetectionRange * 1.5f)
        {
            CurrentAIState = EDinoAIState::Idle;
            TargetActor = nullptr;
        }
    }
    else
    {
        CurrentAIState = EDinoAIState::Idle;
    }
}

void ADinosaurBase::TickAttackBehaviour()
{
    if (!TargetActor || !bIsAggressive)
    {
        CurrentAIState = EDinoAIState::Idle;
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), TargetActor->GetActorLocation());
    if (DistToTarget > DetectionRange * 2.0f)
    {
        // Lost target — return to patrol
        CurrentAIState = EDinoAIState::Patrolling;
        TargetActor = nullptr;
        bIsAggressive = false;
    }
}

void ADinosaurBase::SetAIState(EDinoAIState NewState)
{
    if (CurrentAIState != NewState)
    {
        CurrentAIState = NewState;
        OnAIStateChanged(NewState);
    }
}

float ADinosaurBase::GetHealthPercent() const
{
    return (MaxHealth > 0.0f) ? (CurrentHealth / MaxHealth) : 0.0f;
}

float ADinosaurBase::GetStaminaPercent() const
{
    return (MaxStamina > 0.0f) ? (CurrentStamina / MaxStamina) : 0.0f;
}

bool ADinosaurBase::IsWithinTerritory(const FVector& Location) const
{
    return FVector::Dist(Location, HomeLocation) <= TerritoryRadius;
}

// Blueprint-overridable event stubs
void ADinosaurBase::OnDinosaurDamaged_Implementation(float DamageAmount, AActor* DamageCauser)
{
    // Override in Blueprint for visual/audio feedback
}

void ADinosaurBase::OnDinosaurDied_Implementation()
{
    // Override in Blueprint for death animation, loot spawn, etc.
}

void ADinosaurBase::OnAIStateChanged_Implementation(EDinoAIState NewState)
{
    // Override in Blueprint to drive animation state machine
}
