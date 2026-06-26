#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ============================================================
// UCombat_DinosaurCombatComponent — Implementation
// Agent #12 — Combat & Enemy AI
// ============================================================

UCombat_DinosaurCombatComponent::UCombat_DinosaurCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance friendly
}

void UCombat_DinosaurCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    ApplySpeciesPreset(Species);
    Stats.CurrentHealth = Stats.MaxHealth;
    CurrentState = ECombat_DinoState::Patrolling;
}

void UCombat_DinosaurCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    ScanTimer += DeltaTime;

    // Periodic scan for player
    if (ScanTimer >= ScanInterval)
    {
        ScanTimer = 0.0f;
        ScanForPlayer();
    }

    // Update memory — decay sighting timer
    if (TacticalMemory.bPlayerDetected)
    {
        TacticalMemory.TimeSinceLastSighting += DeltaTime;
        if (TacticalMemory.TimeSinceLastSighting > 10.0f)
        {
            TacticalMemory.bPlayerDetected = false;
            if (CurrentState != ECombat_DinoState::Dead)
            {
                TransitionToState(ECombat_DinoState::Patrolling);
            }
        }
    }

    // State machine dispatch
    switch (CurrentState)
    {
        case ECombat_DinoState::Idle:
        case ECombat_DinoState::Patrolling:
            UpdateIdleState(DeltaTime);
            break;
        case ECombat_DinoState::Alerted:
        case ECombat_DinoState::Stalking:
            UpdateAlertedState(DeltaTime);
            break;
        case ECombat_DinoState::Charging:
        case ECombat_DinoState::Attacking:
            UpdateChargingState(DeltaTime);
            break;
        case ECombat_DinoState::Flanking:
            UpdateFlankingState(DeltaTime);
            break;
        case ECombat_DinoState::Retreating:
        case ECombat_DinoState::Fleeing:
            UpdateRetreatingState(DeltaTime);
            break;
        case ECombat_DinoState::Dead:
            break;
        default:
            break;
    }
}

// ---- State Updates ----

void UCombat_DinosaurCombatComponent::UpdateIdleState(float DeltaTime)
{
    if (TacticalMemory.bPlayerDetected && CurrentTarget.IsValid())
    {
        float Dist = GetDistanceToTarget();
        if (Dist < Stats.DetectionRange)
        {
            TransitionToState(ECombat_DinoState::Alerted);
        }
    }
}

void UCombat_DinosaurCombatComponent::UpdateAlertedState(float DeltaTime)
{
    if (!CurrentTarget.IsValid()) return;

    float Dist = GetDistanceToTarget();

    // Pack hunters flank before charging
    if (Stats.bIsPackHunter && !TacticalMemory.bIsFlankingAssigned && StateTimer > 2.0f)
    {
        TacticalMemory.bIsFlankingAssigned = true;
        float FlankSide = (FMath::RandBool()) ? 1.0f : -1.0f;
        TacticalMemory.AssignedFlankPosition = CalculateFlankPosition(
            CurrentTarget->GetActorLocation(), FlankSide
        );
        TransitionToState(ECombat_DinoState::Flanking);
        return;
    }

    // Solitary hunters charge directly
    if (Dist < Stats.DetectionRange * 0.6f && StateTimer > 1.5f)
    {
        TransitionToState(ECombat_DinoState::Charging);
    }
}

void UCombat_DinosaurCombatComponent::UpdateChargingState(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    // In attack range — deal damage (gameplay system handles actual damage)
    if (Dist < Stats.AttackRange)
    {
        TransitionToState(ECombat_DinoState::Attacking);
        TacticalMemory.AttackAttemptCount++;
    }

    // Health check — flee if low
    if (GetHealthPercent() < Stats.FleeHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }
}

void UCombat_DinosaurCombatComponent::UpdateFlankingState(float DeltaTime)
{
    if (!CurrentTarget.IsValid())
    {
        TransitionToState(ECombat_DinoState::Patrolling);
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner) return;

    float DistToFlankPos = FVector::Dist(Owner->GetActorLocation(), TacticalMemory.AssignedFlankPosition);

    // Reached flank position — now charge
    if (DistToFlankPos < 300.0f || StateTimer > 5.0f)
    {
        TacticalMemory.bIsFlankingAssigned = false;
        TransitionToState(ECombat_DinoState::Charging);
    }
}

void UCombat_DinosaurCombatComponent::UpdateRetreatingState(float DeltaTime)
{
    // After 3 seconds retreating, go idle
    if (StateTimer > 3.0f)
    {
        TransitionToState(ECombat_DinoState::Idle);
    }
}

// ---- Public API ----

void UCombat_DinosaurCombatComponent::SetCombatTarget(AActor* Target)
{
    CurrentTarget = Target;
    if (Target)
    {
        TacticalMemory.bPlayerDetected = true;
        TacticalMemory.TimeSinceLastSighting = 0.0f;
        TacticalMemory.LastKnownPlayerLocation = Target->GetActorLocation();
        if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
        {
            TransitionToState(ECombat_DinoState::Alerted);
        }
    }
}

void UCombat_DinosaurCombatComponent::OnTakeDamage(float DamageAmount)
{
    Stats.CurrentHealth = FMath::Max(0.0f, Stats.CurrentHealth - DamageAmount);

    if (Stats.CurrentHealth <= 0.0f)
    {
        TransitionToState(ECombat_DinoState::Dead);
        return;
    }

    if (GetHealthPercent() < Stats.FleeHealthThreshold)
    {
        TransitionToState(ECombat_DinoState::Fleeing);
    }
    else if (CurrentState == ECombat_DinoState::Idle || CurrentState == ECombat_DinoState::Patrolling)
    {
        // Retaliate when hit
        TransitionToState(ECombat_DinoState::Alerted);
    }
}

float UCombat_DinosaurCombatComponent::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f) return 0.0f;
    return Stats.CurrentHealth / Stats.MaxHealth;
}

bool UCombat_DinosaurCombatComponent::IsHostile() const
{
    return CurrentState == ECombat_DinoState::Charging
        || CurrentState == ECombat_DinoState::Attacking
        || CurrentState == ECombat_DinoState::Flanking
        || CurrentState == ECombat_DinoState::Alerted
        || CurrentState == ECombat_DinoState::Stalking;
}

FVector UCombat_DinosaurCombatComponent::CalculateFlankPosition(const FVector& TargetLocation, float FlankSide) const
{
    AActor* Owner = GetOwner();
    if (!Owner) return TargetLocation;

    FVector ToTarget = (TargetLocation - Owner->GetActorLocation()).GetSafeNormal();
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);
    FVector FlankOffset = Right * (FlankSide * 600.0f) + ToTarget * 400.0f;
    return TargetLocation + FlankOffset;
}

void UCombat_DinosaurCombatComponent::TransitionToState(ECombat_DinoState NewState)
{
    if (CurrentState == ECombat_DinoState::Dead && NewState != ECombat_DinoState::Dead) return;
    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UCombat_DinosaurCombatComponent::ApplySpeciesPreset(ECombat_DinoSpecies InSpecies)
{
    Species = InSpecies;
    switch (InSpecies)
    {
        case ECombat_DinoSpecies::TRex:
            Stats.MaxHealth = 1500.0f;
            Stats.CurrentHealth = 1500.0f;
            Stats.AttackDamage = 200.0f;
            Stats.AttackRange = 350.0f;
            Stats.DetectionRange = 2500.0f;
            Stats.ChargeSpeed = 700.0f;
            Stats.PatrolSpeed = 150.0f;
            Stats.bIsPackHunter = false;
            Stats.FleeHealthThreshold = 0.1f;
            ScanInterval = 0.3f; // T-Rex scans more frequently
            break;

        case ECombat_DinoSpecies::Velociraptor:
            Stats.MaxHealth = 200.0f;
            Stats.CurrentHealth = 200.0f;
            Stats.AttackDamage = 45.0f;
            Stats.AttackRange = 120.0f;
            Stats.DetectionRange = 1800.0f;
            Stats.ChargeSpeed = 1200.0f;
            Stats.PatrolSpeed = 400.0f;
            Stats.bIsPackHunter = true;
            Stats.PackSize = 3;
            Stats.FleeHealthThreshold = 0.25f;
            FlankAngleDegrees = 110.0f;
            ScanInterval = 0.2f; // Raptors are sharp
            break;

        case ECombat_DinoSpecies::Triceratops:
            Stats.MaxHealth = 800.0f;
            Stats.CurrentHealth = 800.0f;
            Stats.AttackDamage = 120.0f;
            Stats.AttackRange = 280.0f;
            Stats.DetectionRange = 1000.0f;
            Stats.ChargeSpeed = 800.0f;
            Stats.PatrolSpeed = 180.0f;
            Stats.bIsPackHunter = false;
            Stats.FleeHealthThreshold = 0.15f;
            break;

        case ECombat_DinoSpecies::Brachiosaurus:
            Stats.MaxHealth = 2000.0f;
            Stats.CurrentHealth = 2000.0f;
            Stats.AttackDamage = 150.0f; // Stomp damage
            Stats.AttackRange = 400.0f;
            Stats.DetectionRange = 800.0f;
            Stats.ChargeSpeed = 400.0f;
            Stats.PatrolSpeed = 120.0f;
            Stats.bIsPackHunter = false;
            Stats.FleeHealthThreshold = 0.05f; // Rarely flees
            break;

        case ECombat_DinoSpecies::Ankylosaurus:
            Stats.MaxHealth = 1000.0f;
            Stats.CurrentHealth = 1000.0f;
            Stats.AttackDamage = 180.0f; // Tail club
            Stats.AttackRange = 250.0f;
            Stats.DetectionRange = 700.0f;
            Stats.ChargeSpeed = 350.0f;
            Stats.PatrolSpeed = 100.0f;
            Stats.bIsPackHunter = false;
            Stats.FleeHealthThreshold = 0.0f; // Never flees
            break;

        default:
            // Generic preset — leave defaults
            break;
    }
}

// ---- Private Helpers ----

void UCombat_DinosaurCombatComponent::ScanForPlayer()
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    UWorld* World = Owner->GetWorld();
    if (!World) return;

    // Find player pawn
    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC) return;

    APawn* PlayerPawn = PC->GetPawn();
    if (!PlayerPawn) return;

    float Dist = FVector::Dist(Owner->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= Stats.DetectionRange)
    {
        TacticalMemory.bPlayerDetected = true;
        TacticalMemory.TimeSinceLastSighting = 0.0f;
        TacticalMemory.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        SetCombatTarget(PlayerPawn);
    }
}

bool UCombat_DinosaurCombatComponent::HasLineOfSightToTarget() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !CurrentTarget.IsValid()) return false;

    UWorld* World = Owner->GetWorld();
    if (!World) return false;

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bBlocked = World->LineTraceSingleByChannel(
        Hit,
        Owner->GetActorLocation() + FVector(0, 0, 100),
        CurrentTarget->GetActorLocation() + FVector(0, 0, 100),
        ECC_Visibility,
        Params
    );

    return !bBlocked || Hit.GetActor() == CurrentTarget.Get();
}

float UCombat_DinosaurCombatComponent::GetDistanceToTarget() const
{
    AActor* Owner = GetOwner();
    if (!Owner || !CurrentTarget.IsValid()) return TNumericLimits<float>::Max();
    return FVector::Dist(Owner->GetActorLocation(), CurrentTarget->GetActorLocation());
}
