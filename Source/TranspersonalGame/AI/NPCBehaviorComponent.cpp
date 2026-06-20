#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for NPC AI
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    ApplySpeciesDefaults();

    if (AActor* Owner = GetOwner())
    {
        PatrolOrigin = Owner->GetActorLocation();
        CurrentPatrolTarget = PatrolOrigin;
    }

    // Start in idle, transition to patrol after short delay
    SetBehaviorState(ENPC_BehaviorState::Idle);
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateMemoryDecay(DeltaTime);
    StateTimer += DeltaTime;

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:        UpdateIdle(DeltaTime);    break;
    case ENPC_BehaviorState::Patrol:      UpdatePatrol(DeltaTime);  break;
    case ENPC_BehaviorState::Alert:       UpdateAlert(DeltaTime);   break;
    case ENPC_BehaviorState::Flee:        UpdateFlee(DeltaTime);    break;
    case ENPC_BehaviorState::Attack:      UpdateAttack(DeltaTime);  break;
    case ENPC_BehaviorState::Graze:       UpdateGraze(DeltaTime);   break;
    case ENPC_BehaviorState::Rest:        UpdateRest(DeltaTime);    break;
    case ENPC_BehaviorState::Investigate: UpdateAlert(DeltaTime);   break;
    default: break;
    }
}

// ── State Machine ──────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::OnStimulusReceived(const FNPC_StimulusEvent& Stimulus)
{
    // Herbivores flee from any strong stimulus; carnivores investigate
    if (Stimulus.Intensity > 0.5f)
    {
        if (bIsHerbivore)
        {
            CurrentThreatLevel = FMath::Clamp(CurrentThreatLevel + Stimulus.Intensity * 0.5f, 0.0f, 1.0f);
            if (CurrentThreatLevel > 0.6f)
            {
                SetBehaviorState(ENPC_BehaviorState::Flee);
            }
            else
            {
                SetBehaviorState(ENPC_BehaviorState::Alert);
            }
        }
        else
        {
            // Carnivore: investigate the stimulus location
            Memory.LastKnownPlayerLocation = Stimulus.Location;
            Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + Stimulus.Intensity * 0.3f, 0.0f, 1.0f);
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
    }
}

void UNPCBehaviorComponent::OnPlayerDetected(FVector PlayerLocation, float Distance)
{
    Memory.LastKnownPlayerLocation = PlayerLocation;
    Memory.TimeSinceLastSighting = 0.0f;
    Memory.bPlayerKnown = true;

    if (bIsHerbivore)
    {
        // Herbivores flee from player
        CurrentThreatLevel = FMath::Clamp(CurrentThreatLevel + 0.4f, 0.0f, 1.0f);
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // Carnivore logic
    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (Distance <= AttackRadius)
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else
    {
        Memory.ThreatLevel = FMath::Clamp(Memory.ThreatLevel + 0.3f, 0.0f, 1.0f);
        CurrentThreatLevel = Memory.ThreatLevel;
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::OnPlayerLost()
{
    Memory.bPlayerKnown = false;

    // Carnivores investigate last known position; herbivores calm down
    if (!bIsHerbivore && Memory.ThreatLevel > 0.3f)
    {
        SetBehaviorState(ENPC_BehaviorState::Investigate);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

// ── Memory ─────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::ForgetPlayer()
{
    Memory.bPlayerKnown = false;
    Memory.ThreatLevel = 0.0f;
    Memory.TimeSinceLastSighting = 0.0f;
    CurrentThreatLevel = 0.0f;
    SetBehaviorState(ENPC_BehaviorState::Patrol);
}

// ── State Updates ──────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::UpdateIdle(float DeltaTime)
{
    // After 3-5 seconds idle, transition to patrol or graze
    if (StateTimer > 3.0f)
    {
        if (bIsHerbivore)
        {
            SetBehaviorState(ENPC_BehaviorState::Graze);
        }
        else
        {
            ChooseNewPatrolTarget();
            SetBehaviorState(ENPC_BehaviorState::Patrol);
        }
    }
}

void UNPCBehaviorComponent::UpdatePatrol(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLocation = Owner->GetActorLocation();
    float DistToTarget = FVector::Dist(CurrentLocation, CurrentPatrolTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached patrol point — wait briefly then choose new target
        if (!bPatrolWaiting)
        {
            bPatrolWaiting = true;
            PatrolWaitTimer = 0.0f;
        }

        PatrolWaitTimer += DeltaTime;
        if (PatrolWaitTimer > 2.0f)
        {
            bPatrolWaiting = false;
            ChooseNewPatrolTarget();
        }
    }

    // Occasionally rest (10% chance per patrol cycle)
    if (StateTimer > 30.0f && FMath::FRand() < 0.1f)
    {
        SetBehaviorState(ENPC_BehaviorState::Rest);
    }
}

void UNPCBehaviorComponent::UpdateAlert(float DeltaTime)
{
    // Alert state: scan for player, escalate or de-escalate
    CurrentThreatLevel = FMath::Max(0.0f, CurrentThreatLevel - DeltaTime * 0.05f);

    if (CurrentThreatLevel <= 0.0f && !Memory.bPlayerKnown)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else if (Memory.bPlayerKnown && IsPlayerInAttackRange())
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
}

void UNPCBehaviorComponent::UpdateFlee(float DeltaTime)
{
    // Flee: reduce threat over time, return to patrol when safe
    CurrentThreatLevel = FMath::Max(0.0f, CurrentThreatLevel - DeltaTime * 0.15f);
    Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - DeltaTime * 0.1f);

    if (CurrentThreatLevel <= 0.1f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::UpdateAttack(float DeltaTime)
{
    if (!Memory.bPlayerKnown)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    if (ShouldFlee())
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // If player escaped attack range, go back to alert/chase
    if (!IsPlayerInAttackRange())
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::UpdateGraze(float DeltaTime)
{
    // Herbivores graze for 10-20 seconds then move to new spot
    if (StateTimer > 12.0f)
    {
        ChooseNewPatrolTarget();
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::UpdateRest(float DeltaTime)
{
    // Rest for 8-15 seconds, then resume patrol
    if (StateTimer > 10.0f)
    {
        ChooseNewPatrolTarget();
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
}

void UNPCBehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    if (Memory.bPlayerKnown)
    {
        Memory.TimeSinceLastSighting += DeltaTime;

        // Forget player after 30 seconds without sighting
        if (Memory.TimeSinceLastSighting > 30.0f)
        {
            Memory.bPlayerKnown = false;
            Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - DeltaTime * MemoryDecayRate);
        }
    }
    else
    {
        Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - DeltaTime * MemoryDecayRate);
    }
}

// ── Helpers ────────────────────────────────────────────────────────────────────

void UNPCBehaviorComponent::ChooseNewPatrolTarget()
{
    // Pick a random point within PatrolRadius of origin
    float Angle = FMath::FRandRange(0.0f, 360.0f);
    float Dist = FMath::FRandRange(PatrolRadius * 0.3f, PatrolRadius);
    float RadAngle = FMath::DegreesToRadians(Angle);

    CurrentPatrolTarget = PatrolOrigin + FVector(
        FMath::Cos(RadAngle) * Dist,
        FMath::Sin(RadAngle) * Dist,
        0.0f
    );
}

bool UNPCBehaviorComponent::IsPlayerInAttackRange() const
{
    AActor* Owner = GetOwner();
    if (!Owner) return false;

    if (!Memory.bPlayerKnown) return false;

    float Dist = FVector::Dist(Owner->GetActorLocation(), Memory.LastKnownPlayerLocation);
    return Dist <= AttackRadius;
}

bool UNPCBehaviorComponent::ShouldFlee() const
{
    return CurrentHealth <= FleeHealthThreshold;
}

void UNPCBehaviorComponent::ApplySpeciesDefaults()
{
    switch (Species)
    {
    case ENPC_DinoSpecies::TRex:
        DetectionRadius = 3000.0f;
        AttackRadius = 350.0f;
        PatrolRadius = 5000.0f;
        FleeHealthThreshold = 0.1f; // T-Rex rarely flees
        bIsHerbivore = false;
        bIsPack = false;
        MemoryDecayRate = 0.05f; // Long memory
        break;

    case ENPC_DinoSpecies::Raptor:
        DetectionRadius = 2500.0f;
        AttackRadius = 250.0f;
        PatrolRadius = 4000.0f;
        FleeHealthThreshold = 0.2f;
        bIsHerbivore = false;
        bIsPack = true; // Pack hunter
        MemoryDecayRate = 0.08f;
        break;

    case ENPC_DinoSpecies::Brachiosaurus:
        DetectionRadius = 1500.0f;
        AttackRadius = 500.0f; // Stomp range
        PatrolRadius = 6000.0f;
        FleeHealthThreshold = 0.3f;
        bIsHerbivore = true;
        bIsPack = false;
        MemoryDecayRate = 0.2f; // Short memory
        break;

    case ENPC_DinoSpecies::Triceratops:
        DetectionRadius = 1800.0f;
        AttackRadius = 400.0f;
        PatrolRadius = 3500.0f;
        FleeHealthThreshold = 0.15f;
        bIsHerbivore = true;
        bIsPack = true; // Herd animal
        MemoryDecayRate = 0.12f;
        break;

    case ENPC_DinoSpecies::Pterodactyl:
        DetectionRadius = 4000.0f; // High vantage point
        AttackRadius = 200.0f;
        PatrolRadius = 8000.0f;
        FleeHealthThreshold = 0.35f; // Fragile — flees easily
        bIsHerbivore = false;
        bIsPack = false;
        MemoryDecayRate = 0.15f;
        break;

    default:
        break;
    }
}
