#include "NPC_DinosaurBehaviorTree.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for behavior updates
}

void UNPC_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();

    // Get owner pawn reference
    OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurBehaviorTree: Owner is not a Pawn"));
        return;
    }

    // Find player pawn
    PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree: Player pawn not found"));
    }

    // Initialize home location
    if (DinosaurMemory.HomeLocation == FVector::ZeroVector)
    {
        DinosaurMemory.HomeLocation = OwnerPawn->GetActorLocation();
    }

    // Generate initial patrol points if none exist
    if (DinosaurMemory.PatrolPoints.Num() == 0)
    {
        GenerateRandomPatrolPoints();
    }

    // Set species-specific stats
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            DinosaurStats.Health = 200.0f;
            DinosaurStats.Aggression = 80.0f;
            DinosaurStats.TerritorialRadius = 5000.0f;
            DinosaurStats.DetectionRadius = 3000.0f;
            DinosaurStats.AttackRange = 400.0f;
            DinosaurStats.MovementSpeed = 300.0f;
            DinosaurStats.ChaseSpeed = 500.0f;
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            DinosaurStats.Health = 80.0f;
            DinosaurStats.Aggression = 90.0f;
            DinosaurStats.TerritorialRadius = 2000.0f;
            DinosaurStats.DetectionRadius = 2500.0f;
            DinosaurStats.AttackRange = 200.0f;
            DinosaurStats.MovementSpeed = 600.0f;
            DinosaurStats.ChaseSpeed = 800.0f;
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            DinosaurStats.Health = 300.0f;
            DinosaurStats.Aggression = 40.0f;
            DinosaurStats.TerritorialRadius = 3000.0f;
            DinosaurStats.DetectionRadius = 1500.0f;
            DinosaurStats.AttackRange = 300.0f;
            DinosaurStats.MovementSpeed = 200.0f;
            DinosaurStats.ChaseSpeed = 350.0f;
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            DinosaurStats.Health = 500.0f;
            DinosaurStats.Aggression = 10.0f;
            DinosaurStats.TerritorialRadius = 4000.0f;
            DinosaurStats.DetectionRadius = 1000.0f;
            DinosaurStats.AttackRange = 500.0f;
            DinosaurStats.MovementSpeed = 150.0f;
            DinosaurStats.ChaseSpeed = 200.0f;
            break;
        default:
            // Default stats already set in struct
            break;
    }

    // Start behavior timers
    GetWorld()->GetTimerManager().SetTimer(StateUpdateTimer, this, &UNPC_DinosaurBehaviorTree::UpdateBehaviorState, StateChangeInterval, true);
    GetWorld()->GetTimerManager().SetTimer(PlayerDetectionTimer, this, &UNPC_DinosaurBehaviorTree::DetectPlayer, PlayerDetectionInterval, true);

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree initialized for %s"), *UEnum::GetValueAsString(Species));
}

void UNPC_DinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerPawn || !PlayerPawn)
        return;

    StateTimer += DeltaTime;

    // Execute current behavior state
    switch (CurrentState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            ExecuteIdleBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Patrolling:
            ExecutePatrolBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Hunting:
            ExecuteHuntingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Chasing:
            ExecuteChasingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Attacking:
            ExecuteAttackingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Fleeing:
            ExecuteFleeingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Feeding:
            ExecuteFeedingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Sleeping:
            ExecuteSleepingBehavior(DeltaTime);
            break;
        case ENPC_DinosaurBehaviorState::Territorial:
            ExecuteTerritorialBehavior(DeltaTime);
            break;
    }

    UpdateMemory();
}

void UNPC_DinosaurBehaviorTree::SetBehaviorState(ENPC_DinosaurBehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurBehaviorState OldState = CurrentState;
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurBehavior: State changed from %s to %s"), 
            *UEnum::GetValueAsString(OldState), 
            *UEnum::GetValueAsString(NewState));
    }
}

void UNPC_DinosaurBehaviorTree::InitializePatrolPoints(const TArray<FVector>& Points)
{
    DinosaurMemory.PatrolPoints = Points;
    CurrentPatrolIndex = 0;
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehavior: Initialized %d patrol points"), Points.Num());
}

void UNPC_DinosaurBehaviorTree::SetHomeLocation(const FVector& Location)
{
    DinosaurMemory.HomeLocation = Location;
}

bool UNPC_DinosaurBehaviorTree::CanSeePlayer() const
{
    if (!PlayerPawn || !OwnerPawn)
        return false;

    float Distance = GetDistanceToPlayer();
    if (Distance > DinosaurStats.DetectionRadius)
        return false;

    // Simple line of sight check
    FVector Start = OwnerPawn->GetActorLocation();
    FVector End = PlayerPawn->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(PlayerPawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
    
    return !bHit; // Can see if no obstruction
}

float UNPC_DinosaurBehaviorTree::GetDistanceToPlayer() const
{
    if (!PlayerPawn || !OwnerPawn)
        return FLT_MAX;

    return FVector::Dist(OwnerPawn->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UNPC_DinosaurBehaviorTree::MoveToLocation(const FVector& TargetLocation)
{
    if (!OwnerPawn)
        return;

    CurrentTargetLocation = TargetLocation;
    bIsMovingToTarget = true;

    // Simple movement towards target
    FVector CurrentLocation = OwnerPawn->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    float Speed = (CurrentState == ENPC_DinosaurBehaviorState::Chasing) ? DinosaurStats.ChaseSpeed : DinosaurStats.MovementSpeed;
    FVector NewLocation = CurrentLocation + (Direction * Speed * GetWorld()->GetDeltaSeconds());
    
    OwnerPawn->SetActorLocation(NewLocation);
    
    // Face movement direction
    if (!Direction.IsZero())
    {
        FRotator NewRotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        OwnerPawn->SetActorRotation(NewRotation);
    }
}

void UNPC_DinosaurBehaviorTree::AttackPlayer()
{
    if (!PlayerPawn || !IsInAttackRange())
        return;

    UE_LOG(LogTemp, Log, TEXT("DinosaurBehavior: Attacking player!"));
    
    // Simple attack implementation
    DinosaurMemory.bIsInCombat = true;
    
    // TODO: Implement actual damage dealing
    // For now, just log the attack
}

void UNPC_DinosaurBehaviorTree::FleeFromThreat(const FVector& ThreatLocation)
{
    if (!OwnerPawn)
        return;

    // Move away from threat
    FVector FleeDirection = (OwnerPawn->GetActorLocation() - ThreatLocation).GetSafeNormal();
    FVector FleeTarget = OwnerPawn->GetActorLocation() + (FleeDirection * 2000.0f);
    
    MoveToLocation(FleeTarget);
    SetBehaviorState(ENPC_DinosaurBehaviorState::Fleeing);
}

void UNPC_DinosaurBehaviorTree::UpdateBehaviorState(float DeltaTime)
{
    if (!PlayerPawn || !OwnerPawn)
        return;

    float PlayerDistance = GetDistanceToPlayer();
    bool bCanSeePlayer = CanSeePlayer();

    // State transition logic based on distance and visibility
    switch (CurrentState)
    {
        case ENPC_DinosaurBehaviorState::Idle:
            if (bCanSeePlayer && PlayerDistance < DinosaurStats.DetectionRadius)
            {
                if (DinosaurStats.Aggression > 50.0f)
                {
                    SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
                }
                else
                {
                    SetBehaviorState(ENPC_DinosaurBehaviorState::Territorial);
                }
            }
            else if (StateTimer > 3.0f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;

        case ENPC_DinosaurBehaviorState::Patrolling:
            if (bCanSeePlayer && PlayerDistance < DinosaurStats.DetectionRadius)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Hunting);
            }
            break;

        case ENPC_DinosaurBehaviorState::Hunting:
            if (PlayerDistance < DinosaurStats.AttackRange)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Attacking);
            }
            else if (PlayerDistance < DinosaurStats.DetectionRadius * 1.5f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Chasing);
            }
            else if (!bCanSeePlayer)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;

        case ENPC_DinosaurBehaviorState::Chasing:
            if (PlayerDistance < DinosaurStats.AttackRange)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Attacking);
            }
            else if (PlayerDistance > DinosaurStats.DetectionRadius * 2.0f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;

        case ENPC_DinosaurBehaviorState::Attacking:
            if (PlayerDistance > DinosaurStats.AttackRange * 1.5f)
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Chasing);
            }
            break;

        case ENPC_DinosaurBehaviorState::Fleeing:
            if (StateTimer > 10.0f) // Flee for 10 seconds
            {
                SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
            }
            break;

        default:
            break;
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteIdleBehavior(float DeltaTime)
{
    // Simple idle behavior - just stand and occasionally look around
    if (FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per tick
    {
        FRotator RandomRotation = FRotator(0, FMath::RandRange(-180.0f, 180.0f), 0);
        OwnerPawn->SetActorRotation(RandomRotation);
    }
}

void UNPC_DinosaurBehaviorTree::ExecutePatrolBehavior(float DeltaTime)
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
        return;

    FVector TargetPoint = DinosaurMemory.PatrolPoints[CurrentPatrolIndex];
    float DistanceToTarget = FVector::Dist(OwnerPawn->GetActorLocation(), TargetPoint);

    if (DistanceToTarget < PatrolPointReachDistance)
    {
        // Move to next patrol point
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % DinosaurMemory.PatrolPoints.Num();
    }
    else
    {
        MoveToLocation(TargetPoint);
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteHuntingBehavior(float DeltaTime)
{
    if (PlayerPawn)
    {
        DinosaurMemory.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        DinosaurMemory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
        MoveToLocation(DinosaurMemory.LastKnownPlayerLocation);
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteChasingBehavior(float DeltaTime)
{
    if (PlayerPawn)
    {
        MoveToLocation(PlayerPawn->GetActorLocation());
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteAttackingBehavior(float DeltaTime)
{
    AttackPlayer();
}

void UNPC_DinosaurBehaviorTree::ExecuteFleeingBehavior(float DeltaTime)
{
    // Continue fleeing towards home location
    MoveToLocation(DinosaurMemory.HomeLocation);
}

void UNPC_DinosaurBehaviorTree::ExecuteFeedingBehavior(float DeltaTime)
{
    // Simple feeding behavior - stay in place and "eat"
    DinosaurStats.Hunger = FMath::Max(0.0f, DinosaurStats.Hunger - (DeltaTime * 10.0f));
    
    if (DinosaurStats.Hunger <= 0.0f)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteSleepingBehavior(float DeltaTime)
{
    // Restore stamina while sleeping
    DinosaurStats.Stamina = FMath::Min(100.0f, DinosaurStats.Stamina + (DeltaTime * 20.0f));
    
    if (DinosaurStats.Stamina >= 100.0f)
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Idle);
    }
}

void UNPC_DinosaurBehaviorTree::ExecuteTerritorialBehavior(float DeltaTime)
{
    if (IsPlayerInTerritory())
    {
        // Move towards player to intimidate
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            FVector Direction = (PlayerLocation - OwnerPawn->GetActorLocation()).GetSafeNormal();
            FVector IntimidatePosition = PlayerLocation - (Direction * (DinosaurStats.AttackRange * 2.0f));
            MoveToLocation(IntimidatePosition);
        }
    }
    else
    {
        SetBehaviorState(ENPC_DinosaurBehaviorState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorTree::DetectPlayer()
{
    if (CanSeePlayer())
    {
        DinosaurMemory.bHasSeenPlayer = true;
        DinosaurMemory.LastKnownPlayerLocation = PlayerPawn->GetActorLocation();
        DinosaurMemory.LastPlayerSightingTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_DinosaurBehaviorTree::UpdateMemory()
{
    // Update hunger over time
    DinosaurStats.Hunger = FMath::Min(100.0f, DinosaurStats.Hunger + (GetWorld()->GetDeltaSeconds() * 0.5f));
    
    // Update stamina based on activity
    float StaminaDrain = 0.0f;
    switch (CurrentState)
    {
        case ENPC_DinosaurBehaviorState::Chasing:
            StaminaDrain = 2.0f;
            break;
        case ENPC_DinosaurBehaviorState::Attacking:
            StaminaDrain = 3.0f;
            break;
        case ENPC_DinosaurBehaviorState::Fleeing:
            StaminaDrain = 2.5f;
            break;
        default:
            StaminaDrain = 0.1f;
            break;
    }
    
    DinosaurStats.Stamina = FMath::Max(0.0f, DinosaurStats.Stamina - (GetWorld()->GetDeltaSeconds() * StaminaDrain));
    
    // If stamina is low, consider resting
    if (DinosaurStats.Stamina < 20.0f && CurrentState != ENPC_DinosaurBehaviorState::Sleeping)
    {
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance to sleep when tired
        {
            SetBehaviorState(ENPC_DinosaurBehaviorState::Sleeping);
        }
    }
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint()
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
        return OwnerPawn->GetActorLocation();

    return DinosaurMemory.PatrolPoints[CurrentPatrolIndex];
}

bool UNPC_DinosaurBehaviorTree::IsInAttackRange() const
{
    return GetDistanceToPlayer() <= DinosaurStats.AttackRange;
}

bool UNPC_DinosaurBehaviorTree::IsPlayerInTerritory() const
{
    if (!PlayerPawn)
        return false;

    float DistanceFromHome = FVector::Dist(PlayerPawn->GetActorLocation(), DinosaurMemory.HomeLocation);
    return DistanceFromHome <= DinosaurStats.TerritorialRadius;
}

void UNPC_DinosaurBehaviorTree::GenerateRandomPatrolPoints()
{
    DinosaurMemory.PatrolPoints.Empty();
    
    FVector HomeLocation = DinosaurMemory.HomeLocation;
    int32 NumPoints = FMath::RandRange(3, 6);
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = (360.0f / NumPoints) * i + FMath::RandRange(-30.0f, 30.0f);
        float Distance = FMath::RandRange(DinosaurStats.TerritorialRadius * 0.3f, DinosaurStats.TerritorialRadius * 0.7f);
        
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
        
        DinosaurMemory.PatrolPoints.Add(HomeLocation + Offset);
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehavior: Generated %d patrol points"), NumPoints);
}