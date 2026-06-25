#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance friendly
    CurrentHealth = MaxHealth;
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    CurrentHealth = MaxHealth;
    HomeLocation = GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;

    // Auto-generate patrol points if none assigned
    if (PatrolPoints.Num() == 0 && DefaultState == ENPC_BehaviorState::Patrol)
    {
        GenerateRandomPatrolPoints(4);
    }

    SetBehaviorState(DefaultState);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateBehaviorTick(DeltaTime);
}

void UNPCBehaviorComponent::UpdateBehaviorTick(float DeltaTime)
{
    if (CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    // Update memory decay
    UpdateMemory(DeltaTime);

    // Evaluate state transitions
    EvaluateStateTransition();

    // Patrol wait timer
    if (bWaitingAtPatrolPoint)
    {
        PatrolWaitTimer -= DeltaTime;
        if (PatrolWaitTimer <= 0.0f)
        {
            bWaitingAtPatrolPoint = false;
        }
    }
}

void UNPCBehaviorComponent::UpdateMemory(float DeltaTime)
{
    if (bPlayerDetected)
    {
        TimeSinceLastPlayerSight = 0.0f;
    }
    else if (TimeSinceLastPlayerSight < MemoryDuration)
    {
        TimeSinceLastPlayerSight += DeltaTime;
    }

    // Memory fades after MemoryDuration seconds
    if (TimeSinceLastPlayerSight >= MemoryDuration)
    {
        PlayerMemory.bIsThreat = false;
        PlayerMemory.ThreatLevel = 0.0f;
        bIsAlerted = false;
    }
}

void UNPCBehaviorComponent::EvaluateStateTransition()
{
    if (CurrentState == ENPC_BehaviorState::Dead || CurrentState == ENPC_BehaviorState::Attack)
    {
        return;
    }

    // Flee if low health
    if (GetHealthPercent() < FleeHealthThreshold && CurrentState != ENPC_BehaviorState::Flee)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
        return;
    }

    // If player detected and in attack range → attack
    if (bPlayerDetected && TrackedPlayer)
    {
        if (IsPlayerInAttackRange(TrackedPlayer))
        {
            SetBehaviorState(ENPC_BehaviorState::Attack);
            return;
        }
        // Player detected but not in range → chase
        if (CurrentState != ENPC_BehaviorState::Chase && CurrentState != ENPC_BehaviorState::Alert)
        {
            SetBehaviorState(ENPC_BehaviorState::Chase);
        }
        return;
    }

    // Memory still active but player not visible → alert
    if (PlayerMemory.bIsThreat && TimeSinceLastPlayerSight < MemoryDuration && CurrentState != ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
        return;
    }

    // Return to default state if no threats
    if (!bIsAlerted && !bPlayerDetected && CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(DefaultState);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    ENPC_BehaviorState PreviousState = CurrentState;
    CurrentState = NewState;

    UE_LOG(LogTemp, Log, TEXT("[NPCBehavior] %s: %d -> %d"),
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"),
        (int32)PreviousState,
        (int32)NewState);

    // State entry logic
    switch (NewState)
    {
    case ENPC_BehaviorState::Alert:
        bIsAlerted = true;
        break;

    case ENPC_BehaviorState::Chase:
        bIsAlerted = true;
        break;

    case ENPC_BehaviorState::Idle:
    case ENPC_BehaviorState::Patrol:
        bIsAlerted = false;
        bPlayerDetected = false;
        break;

    case ENPC_BehaviorState::Dead:
        bIsAlerted = false;
        bPlayerDetected = false;
        SetComponentTickEnabled(false);
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::OnPlayerDetected(AActor* Player, FVector PlayerLocation)
{
    if (!Player)
    {
        return;
    }

    TrackedPlayer = Player;
    bPlayerDetected = true;
    bIsAlerted = true;
    TimeSinceLastPlayerSight = 0.0f;

    // Update memory
    PlayerMemory.LastKnownLocation = PlayerLocation;
    PlayerMemory.TimeStamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    PlayerMemory.ThreatLevel = FMath::Clamp(PlayerMemory.ThreatLevel + 0.25f, 0.0f, 1.0f);
    PlayerMemory.bIsThreat = true;

    // Trigger state transition
    if (IsPlayerInAttackRange(Player))
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
    }
}

void UNPCBehaviorComponent::OnPlayerLost()
{
    bPlayerDetected = false;
    TrackedPlayer = nullptr;

    // Don't immediately forget — memory lingers
    if (PlayerMemory.bIsThreat)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPCBehaviorComponent::TakeDamageNPC(float DamageAmount)
{
    if (CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    CurrentHealth = FMath::Clamp(CurrentHealth - DamageAmount, 0.0f, MaxHealth);

    // Taking damage raises threat level
    PlayerMemory.ThreatLevel = FMath::Clamp(PlayerMemory.ThreatLevel + 0.5f, 0.0f, 1.0f);
    bIsAlerted = true;

    if (CurrentHealth <= 0.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Dead);
    }
    else if (GetHealthPercent() < FleeHealthThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (CurrentState != ENPC_BehaviorState::Chase && CurrentState != ENPC_BehaviorState::Attack)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

bool UNPCBehaviorComponent::IsPlayerInSightCone(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector PlayerLoc = Player->GetActorLocation();
    float Distance = FVector::Dist(OwnerLoc, PlayerLoc);

    if (Distance > SightRadius)
    {
        return false;
    }

    // Check angle
    FVector ToPlayer = (PlayerLoc - OwnerLoc).GetSafeNormal();
    FVector Forward = GetOwner()->GetActorForwardVector();
    float DotProduct = FVector::DotProduct(Forward, ToPlayer);
    float AngleDeg = FMath::RadiansToDegrees(FMath::Acos(DotProduct));

    return AngleDeg <= (SightAngleDegrees * 0.5f);
}

bool UNPCBehaviorComponent::IsPlayerInAttackRange(AActor* Player) const
{
    if (!Player || !GetOwner())
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= AttackRadius;
}

float UNPCBehaviorComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return CurrentHealth / MaxHealth;
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return HomeLocation;
    }

    FNPC_PatrolPoint& Point = PatrolPoints[CurrentPatrolIndex];
    FVector Target = Point.Location;

    // Advance to next point
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();

    // Set wait timer
    PatrolWaitTimer = Point.WaitTime;
    bWaitingAtPatrolPoint = Point.WaitTime > 0.0f;

    return Target;
}

void UNPCBehaviorComponent::GenerateRandomPatrolPoints(int32 Count)
{
    PatrolPoints.Empty();

    for (int32 i = 0; i < Count; i++)
    {
        FNPC_PatrolPoint Point;
        float Angle = (360.0f / Count) * i;
        float RadAngle = FMath::DegreesToRadians(Angle);
        float Radius = PatrolRadius * FMath::RandRange(0.4f, 1.0f);

        Point.Location = HomeLocation + FVector(
            FMath::Cos(RadAngle) * Radius,
            FMath::Sin(RadAngle) * Radius,
            0.0f
        );
        Point.WaitTime = FMath::RandRange(1.5f, 4.0f);
        Point.bIsWaypoint = true;

        PatrolPoints.Add(Point);
    }

    UE_LOG(LogTemp, Log, TEXT("[NPCBehavior] Generated %d patrol points for %s"),
        Count,
        GetOwner() ? *GetOwner()->GetActorLabel() : TEXT("Unknown"));
}
