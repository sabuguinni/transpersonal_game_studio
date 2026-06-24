#include "NPCBehaviorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick — sufficient for NPC logic
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Cache player reference
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (PlayerChar)
    {
        CachedPlayerActor = PlayerChar;
    }

    // Start in patrol if waypoints exist, otherwise idle
    if (PatrolData.WaypointLocations.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    ScanTimer += DeltaTime;

    // Periodic threat scan
    if (ScanTimer >= ScanInterval)
    {
        ScanTimer = 0.0f;
        ScanForThreats();
    }

    UpdateBehaviorLogic(DeltaTime);
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateTimer = 0.0f;

    // Adjust movement speed based on state
    AActor* Owner = GetOwner();
    if (!Owner) return;

    ACharacter* OwnerChar = Cast<ACharacter>(Owner);
    if (!OwnerChar) return;

    UCharacterMovementComponent* Movement = OwnerChar->GetCharacterMovement();
    if (!Movement) return;

    switch (NewState)
    {
    case ENPC_BehaviorState::Patrol:
        Movement->MaxWalkSpeed = PatrolSpeed;
        break;
    case ENPC_BehaviorState::Chase:
    case ENPC_BehaviorState::Flee:
        Movement->MaxWalkSpeed = ChaseSpeed;
        break;
    case ENPC_BehaviorState::Idle:
    case ENPC_BehaviorState::Investigate:
        Movement->MaxWalkSpeed = PatrolSpeed * 0.5f;
        break;
    case ENPC_BehaviorState::Attack:
        Movement->MaxWalkSpeed = ChaseSpeed * 0.8f;
        break;
    case ENPC_BehaviorState::Dead:
        Movement->MaxWalkSpeed = 0.0f;
        Movement->DisableMovement();
        break;
    default:
        break;
    }
}

void UNPCBehaviorComponent::RegisterThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor) return;

    ActiveThreat.ThreatActor = ThreatActor;
    ActiveThreat.LastKnownLocation = ThreatActor->GetActorLocation();
    ActiveThreat.ThreatLevel = ThreatLevel;
    ActiveThreat.TimeLastSeen = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    // Decide response based on health and threat level
    if (CurrentHealth <= FleeHealthThreshold)
    {
        SetBehaviorState(ENPC_BehaviorState::Flee);
    }
    else if (IsPlayerInAttackRange())
    {
        SetBehaviorState(ENPC_BehaviorState::Attack);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Chase);
    }
}

void UNPCBehaviorComponent::ClearThreat()
{
    ActiveThreat.ThreatActor = nullptr;
    ActiveThreat.ThreatLevel = 0.0f;

    // Return to patrol or idle
    if (PatrolData.WaypointLocations.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrol);
    }
    else
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

bool UNPCBehaviorComponent::IsPlayerInDetectionRange() const
{
    float Dist = GetDistanceToPlayer();
    return Dist > 0.0f && Dist <= DetectionRadius;
}

bool UNPCBehaviorComponent::IsPlayerInAttackRange() const
{
    float Dist = GetDistanceToPlayer();
    return Dist > 0.0f && Dist <= AttackRadius;
}

FVector UNPCBehaviorComponent::GetNextPatrolWaypoint()
{
    if (PatrolData.WaypointLocations.Num() == 0)
    {
        return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    }

    int32 Idx = PatrolData.CurrentWaypointIndex % PatrolData.WaypointLocations.Num();
    return PatrolData.WaypointLocations[Idx];
}

void UNPCBehaviorComponent::AdvancePatrolWaypoint()
{
    if (PatrolData.WaypointLocations.Num() == 0) return;
    PatrolData.CurrentWaypointIndex = (PatrolData.CurrentWaypointIndex + 1) % PatrolData.WaypointLocations.Num();
}

void UNPCBehaviorComponent::UpdateBehaviorLogic(float DeltaTime)
{
    if (CurrentState == ENPC_BehaviorState::Dead) return;

    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        // If player enters detection range, investigate
        if (IsPlayerInDetectionRange())
        {
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        break;

    case ENPC_BehaviorState::Patrol:
        // Check for player
        if (IsPlayerInDetectionRange())
        {
            if (CachedPlayerActor)
            {
                RegisterThreat(CachedPlayerActor, 1.0f);
            }
        }
        break;

    case ENPC_BehaviorState::Investigate:
        // After 3 seconds investigating without finding player, return to patrol
        if (StateTimer > 3.0f && !IsPlayerInDetectionRange())
        {
            ClearThreat();
        }
        else if (IsPlayerInAttackRange())
        {
            SetBehaviorState(ENPC_BehaviorState::Attack);
        }
        break;

    case ENPC_BehaviorState::Chase:
        // Update last known location
        if (ActiveThreat.ThreatActor)
        {
            ActiveThreat.LastKnownLocation = ActiveThreat.ThreatActor->GetActorLocation();
            ActiveThreat.TimeLastSeen = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

            if (IsPlayerInAttackRange())
            {
                SetBehaviorState(ENPC_BehaviorState::Attack);
            }
        }
        else
        {
            // Lost the target — investigate last known location
            SetBehaviorState(ENPC_BehaviorState::Investigate);
        }
        break;

    case ENPC_BehaviorState::Attack:
        // If player moved out of attack range, chase again
        if (!IsPlayerInAttackRange())
        {
            if (IsPlayerInDetectionRange())
            {
                SetBehaviorState(ENPC_BehaviorState::Chase);
            }
            else
            {
                ClearThreat();
            }
        }
        break;

    case ENPC_BehaviorState::Flee:
        // Keep fleeing until player is out of detection range
        if (!IsPlayerInDetectionRange())
        {
            ClearThreat();
        }
        break;

    default:
        break;
    }
}

void UNPCBehaviorComponent::ScanForThreats()
{
    if (!CachedPlayerActor)
    {
        ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
        if (PlayerChar)
        {
            CachedPlayerActor = PlayerChar;
        }
    }

    // Already in combat states — no need to re-scan
    if (CurrentState == ENPC_BehaviorState::Chase ||
        CurrentState == ENPC_BehaviorState::Attack ||
        CurrentState == ENPC_BehaviorState::Dead)
    {
        return;
    }

    if (IsPlayerInDetectionRange() && CachedPlayerActor)
    {
        RegisterThreat(CachedPlayerActor, 1.0f);
    }
}

float UNPCBehaviorComponent::GetDistanceToPlayer() const
{
    if (!CachedPlayerActor || !GetOwner()) return -1.0f;
    return FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
}
