#include "DinosaurBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UDinosaurBehaviorComponent::UDinosaurBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for AI
}

void UDinosaurBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    CurrentState = ENPC_DinoState::Patrolling;

    // Generate random patrol points around spawn if none defined
    if (PatrolPoints.Num() == 0 && GetOwner())
    {
        FVector Origin = GetOwner()->GetActorLocation();
        const int32 NumPoints = 4;
        for (int32 i = 0; i < NumPoints; ++i)
        {
            float Angle = (360.0f / NumPoints) * i;
            float Rad = FMath::DegreesToRadians(Angle);
            FNPC_PatrolPoint Pt;
            Pt.Location = Origin + FVector(
                FMath::Cos(Rad) * PatrolRadius * 0.5f,
                FMath::Sin(Rad) * PatrolRadius * 0.5f,
                0.0f
            );
            Pt.WaitTime = FMath::RandRange(1.5f, 4.0f);
            Pt.bLookAround = (i % 2 == 0);
            PatrolPoints.Add(Pt);
        }
    }
}

void UDinosaurBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TickMemory(DeltaTime);
    TickHunger(DeltaTime);
    UpdateStateFromPerception();
}

// ─── State Management ─────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::SetState(ENPC_DinoState NewState)
{
    if (NewState == CurrentState) return;
    PreviousState = CurrentState;
    CurrentState = NewState;
}

// ─── Perception ───────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::OnPlayerDetected(AActor* Player, float Distance, bool bIsSprinting)
{
    if (!Player) return;

    TrackedPlayer = Player;
    bPlayerDetected = true;
    bPlayerIsSprinting = bIsSprinting;

    // Update memory
    PlayerMemory.LastKnownLocation = Player->GetActorLocation();
    PlayerMemory.TimeSinceLastSeen = 0.0f;
    PlayerMemory.bIsPlayerTarget = true;

    // Escalate threat based on distance and sprint state
    float EffectiveSight = GetEffectiveSightRadius();
    float NormDist = FMath::Clamp(Distance / EffectiveSight, 0.0f, 1.0f);

    if (NormDist < 0.2f || bIsSprinting)
    {
        PlayerMemory.ThreatLevel = ENPC_ThreatLevel::Critical;
    }
    else if (NormDist < 0.5f)
    {
        PlayerMemory.ThreatLevel = ENPC_ThreatLevel::High;
    }
    else if (NormDist < 0.75f)
    {
        PlayerMemory.ThreatLevel = ENPC_ThreatLevel::Medium;
    }
    else
    {
        PlayerMemory.ThreatLevel = ENPC_ThreatLevel::Low;
    }

    // Update range flags
    bPlayerInChaseRange = (Distance <= ChaseRadius);
    bPlayerInAttackRange = (Distance <= AttackRadius);

    // State transitions
    if (bPlayerInAttackRange)
    {
        SetState(ENPC_DinoState::Attacking);
    }
    else if (bPlayerInChaseRange)
    {
        SetState(ENPC_DinoState::Chasing);
    }
    else
    {
        SetState(ENPC_DinoState::Alerted);
    }
}

void UDinosaurBehaviorComponent::OnPlayerLost()
{
    bPlayerDetected = false;
    bPlayerInChaseRange = false;
    bPlayerInAttackRange = false;
    bPlayerIsSprinting = false;
    bPlayerIsFleeing = false;

    // Keep memory alive — dino remembers last known position
    if (CurrentState == ENPC_DinoState::Chasing || CurrentState == ENPC_DinoState::Attacking)
    {
        SetState(ENPC_DinoState::Alerted);
    }
}

float UDinosaurBehaviorComponent::GetEffectiveSightRadius() const
{
    float Base = PerceptionConfig.SightRadius;

    if (bPlayerIsSprinting)
    {
        return Base * PerceptionConfig.SprintDetectionMultiplier;
    }

    // Herbivores have wider passive sight
    if (Species == ENPC_DinoSpecies::Brachiosaurus)
    {
        return Base * 1.2f;
    }

    return Base;
}

// ─── Combat ───────────────────────────────────────────────────────────────────

bool UDinosaurBehaviorComponent::CanAttack() const
{
    if (!GetWorld()) return false;
    float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastAttackTime) >= AttackCooldown;
}

void UDinosaurBehaviorComponent::PerformAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        Target,
        AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );
}

// ─── Stats ────────────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::ApplyDamage(float DamageAmount)
{
    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        SetState(ENPC_DinoState::Fleeing);
    }
    else if (CurrentHealth / MaxHealth < 0.25f)
    {
        // Low health — herbivores flee, carnivores may escalate
        if (Species == ENPC_DinoSpecies::Brachiosaurus || Species == ENPC_DinoSpecies::Triceratops)
        {
            SetState(ENPC_DinoState::Fleeing);
        }
    }
}

float UDinosaurBehaviorComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}

// ─── State Queries ────────────────────────────────────────────────────────────

bool UDinosaurBehaviorComponent::IsAggressive() const
{
    return Species == ENPC_DinoSpecies::TRex
        || Species == ENPC_DinoSpecies::Raptor;
}

bool UDinosaurBehaviorComponent::IsPassive() const
{
    return Species == ENPC_DinoSpecies::Brachiosaurus;
}

// ─── Patrol ───────────────────────────────────────────────────────────────────

FVector UDinosaurBehaviorComponent::GetNextPatrolPoint() const
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }
    return PatrolPoints[CurrentPatrolIndex].Location;
}

void UDinosaurBehaviorComponent::AdvancePatrolIndex()
{
    if (PatrolPoints.Num() == 0) return;
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
}

// ─── Private Ticks ────────────────────────────────────────────────────────────

void UDinosaurBehaviorComponent::TickMemory(float DeltaTime)
{
    if (!bPlayerDetected && PlayerMemory.bIsPlayerTarget)
    {
        PlayerMemory.TimeSinceLastSeen += DeltaTime;

        if (PlayerMemory.TimeSinceLastSeen >= MemoryDuration)
        {
            // Memory expired — forget player
            PlayerMemory = FNPC_DinoMemoryEntry();
            TrackedPlayer = nullptr;

            if (CurrentState == ENPC_DinoState::Alerted)
            {
                SetState(ENPC_DinoState::Patrolling);
            }
        }
    }
}

void UDinosaurBehaviorComponent::TickHunger(float DeltaTime)
{
    Hunger = FMath::Max(0.0f, Hunger - HungerDecayRate * DeltaTime);

    // Starving carnivores become more aggressive — lower chase threshold
    if (Hunger < 0.2f && IsAggressive())
    {
        ChaseRadius = FMath::Min(ChaseRadius * 1.01f, 6000.0f);
    }
}

void UDinosaurBehaviorComponent::UpdateStateFromPerception()
{
    // If player is detected and we're just patrolling, escalate
    if (bPlayerDetected && CurrentState == ENPC_DinoState::Patrolling)
    {
        if (IsAggressive())
        {
            SetState(ENPC_DinoState::Chasing);
        }
        else
        {
            SetState(ENPC_DinoState::Alerted);
        }
    }

    // If fleeing player is detected by aggressive dino — escalate to critical chase
    if (bPlayerIsFleeing && IsAggressive() && bPlayerDetected)
    {
        if (CurrentState != ENPC_DinoState::Attacking)
        {
            SetState(ENPC_DinoState::Chasing);
        }
    }
}
