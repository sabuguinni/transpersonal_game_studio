#include "DinosaurBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

ADinosaurBase::ADinosaurBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default capsule for a medium-sized dinosaur
    GetCapsuleComponent()->InitCapsuleSize(60.0f, 120.0f);

    // Movement defaults — overridden per species in InitializeSpeciesDefaults()
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 360.0f, 0.0f);
        MoveComp->GravityScale = 1.0f;
        MoveComp->JumpZVelocity = 400.0f;
        MoveComp->AirControl = 0.1f;
    }

    bUseControllerRotationYaw = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
}

void ADinosaurBase::BeginPlay()
{
    Super::BeginPlay();

    // Record spawn location as territory center
    TerritoryCenter = GetActorLocation();

    // Apply species-specific defaults (overridden in subclasses)
    InitializeSpeciesDefaults();

    // Sync movement speed from stats
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    // Determine initial aggression from species data
    bIsAggressive = (SpeciesData.Diet == EEng_DinoDiet::Carnivore &&
                     SpeciesData.AggressionLevel > 0.5f);
}

void ADinosaurBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!IsAlive())
    {
        return;
    }

    TickHunger(DeltaTime);
    TickBehavior(DeltaTime);
}

// ─── Stats ───────────────────────────────────────────────────────────────────

void ADinosaurBase::ApplyDamage(float DamageAmount)
{
    if (!IsAlive() || DamageAmount <= 0.0f)
    {
        return;
    }

    DinoStats.CurrentHealth = FMath::Max(0.0f, DinoStats.CurrentHealth - DamageAmount);

    if (!IsAlive())
    {
        SetBehaviorState(EEng_DinoBehaviorState::Dead);
        OnDeath();
    }
}

void ADinosaurBase::Heal(float HealAmount)
{
    if (!IsAlive() || HealAmount <= 0.0f)
    {
        return;
    }

    DinoStats.CurrentHealth = FMath::Min(DinoStats.MaxHealth,
                                          DinoStats.CurrentHealth + HealAmount);
}

bool ADinosaurBase::IsAlive() const
{
    return DinoStats.CurrentHealth > 0.0f;
}

float ADinosaurBase::GetHealthPercent() const
{
    if (DinoStats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return DinoStats.CurrentHealth / DinoStats.MaxHealth;
}

bool ADinosaurBase::IsHungry() const
{
    return DinoStats.Hunger < 30.0f;
}

// ─── Behavior State ───────────────────────────────────────────────────────────

void ADinosaurBase::SetBehaviorState(EEng_DinoBehaviorState NewState)
{
    if (CurrentBehaviorState == NewState)
    {
        return;
    }

    CurrentBehaviorState = NewState;
    OnBehaviorStateChanged(NewState);

    // Adjust movement speed based on state
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        switch (NewState)
        {
            case EEng_DinoBehaviorState::Hunting:
            case EEng_DinoBehaviorState::Attacking:
                MoveComp->MaxWalkSpeed = DinoStats.SprintSpeed;
                break;
            case EEng_DinoBehaviorState::Fleeing:
                MoveComp->MaxWalkSpeed = DinoStats.SprintSpeed * 1.2f;
                break;
            case EEng_DinoBehaviorState::Sleeping:
            case EEng_DinoBehaviorState::Eating:
                MoveComp->MaxWalkSpeed = 0.0f;
                break;
            default:
                MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
                break;
        }
    }
}

void ADinosaurBase::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        OnTargetDetected(NewTarget);
        if (SpeciesData.Diet == EEng_DinoDiet::Carnivore)
        {
            SetBehaviorState(EEng_DinoBehaviorState::Hunting);
        }
    }
}

void ADinosaurBase::ClearTarget()
{
    CurrentTarget = nullptr;
    if (CurrentBehaviorState == EEng_DinoBehaviorState::Hunting ||
        CurrentBehaviorState == EEng_DinoBehaviorState::Attacking)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Patrolling);
    }
}

// ─── Territory ────────────────────────────────────────────────────────────────

bool ADinosaurBase::IsInTerritory(const FVector& Location) const
{
    if (!bHasTerritory)
    {
        return true;
    }
    return FVector::Dist(Location, TerritoryCenter) <= DinoStats.TerritoryRadius;
}

FVector ADinosaurBase::GetRandomPatrolPoint() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return TerritoryCenter;
    }

    // Random point within patrol radius, clamped to territory
    const float Radius = FMath::Min(PatrolRadius, DinoStats.TerritoryRadius);
    const float Angle = FMath::RandRange(0.0f, 360.0f);
    const float Distance = FMath::RandRange(200.0f, Radius);

    FVector Offset(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
        0.0f
    );

    FVector Target = TerritoryCenter + Offset;

    // Project to navigation mesh if available
    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if (NavSys && NavSys->ProjectPointToNavigation(Target, NavLoc, FVector(200.0f, 200.0f, 200.0f)))
    {
        return NavLoc.Location;
    }

    return Target;
}

// ─── Internal Tick ────────────────────────────────────────────────────────────

void ADinosaurBase::TickHunger(float DeltaTime)
{
    HungerAccumulator += DeltaTime;

    // Decay hunger every second
    if (HungerAccumulator >= 1.0f)
    {
        HungerAccumulator = 0.0f;
        DinoStats.Hunger = FMath::Max(0.0f,
            DinoStats.Hunger - DinoStats.HungerDecayRate * DeltaTime);

        // Starving damages health slowly
        if (DinoStats.Hunger <= 0.0f)
        {
            ApplyDamage(1.0f);
        }
    }
}

void ADinosaurBase::TickBehavior(float DeltaTime)
{
    // Basic idle → patrol transition
    if (CurrentBehaviorState == EEng_DinoBehaviorState::Idle)
    {
        TimeSinceLastPatrol += DeltaTime;
        if (TimeSinceLastPatrol >= PatrolWaitTime)
        {
            TimeSinceLastPatrol = 0.0f;
            SetBehaviorState(EEng_DinoBehaviorState::Patrolling);
        }
    }

    // If hunting but no target, return to patrol
    if (CurrentBehaviorState == EEng_DinoBehaviorState::Hunting && !CurrentTarget)
    {
        SetBehaviorState(EEng_DinoBehaviorState::Patrolling);
    }

    // Hungry carnivores become more aggressive
    if (IsHungry() && SpeciesData.Diet == EEng_DinoDiet::Carnivore)
    {
        bIsAggressive = true;
    }
}

void ADinosaurBase::InitializeSpeciesDefaults()
{
    // Base implementation — subclasses override to set species-specific stats
    // e.g., ATyrannosaurusRex::InitializeSpeciesDefaults() sets MaxHealth=1000, AttackDamage=150
}
