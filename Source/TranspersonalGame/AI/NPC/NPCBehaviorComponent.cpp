#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    CurrentState = ENPC_BehaviorState::Idle;
    DetectionRadius = 1500.0f;
    FleeRadius = 800.0f;
    MovementSpeed = 300.0f;
    PatrolRadius = 2000.0f;
    
    Courage = 0.5f;
    Curiosity = 0.7f;
    Sociability = 0.6f;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize patrol points around spawn location
    if (AActor* Owner = GetOwner())
    {
        FVector SpawnLocation = Owner->GetActorLocation();
        BehaviorMemory.PatrolPoints.Empty();
        
        // Create 4 patrol points in a square pattern
        float PatrolDistance = PatrolRadius * 0.7f;
        BehaviorMemory.PatrolPoints.Add(SpawnLocation + FVector(PatrolDistance, PatrolDistance, 0));
        BehaviorMemory.PatrolPoints.Add(SpawnLocation + FVector(-PatrolDistance, PatrolDistance, 0));
        BehaviorMemory.PatrolPoints.Add(SpawnLocation + FVector(-PatrolDistance, -PatrolDistance, 0));
        BehaviorMemory.PatrolPoints.Add(SpawnLocation + FVector(PatrolDistance, -PatrolDistance, 0));
        
        BehaviorMemory.CurrentPatrolIndex = 0;
    }
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update memory timers
    BehaviorMemory.TimeSincePlayerSeen += DeltaTime;

    // State machine
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENPC_BehaviorState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ENPC_BehaviorState::Investigating:
            HandleInvestigatingState(DeltaTime);
            break;
        case ENPC_BehaviorState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        default:
            break;
    }

    // Check for player detection
    if (CanSeePlayer())
    {
        ReactToPlayer();
    }
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed state to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
               (int32)NewState);
    }
}

bool UNPC_BehaviorComponent::CanSeePlayer()
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner())
    {
        return false;
    }

    float Distance = GetDistanceToPlayer();
    if (Distance > DetectionRadius)
    {
        return false;
    }

    // Simple line trace for line of sight
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = Player->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // Can see if no obstruction
}

void UNPC_BehaviorComponent::UpdatePatrol()
{
    if (BehaviorMemory.PatrolPoints.Num() == 0)
    {
        return;
    }

    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetPoint = BehaviorMemory.PatrolPoints[BehaviorMemory.CurrentPatrolIndex];
    
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPoint);
    
    if (DistanceToTarget < 200.0f) // Close enough to patrol point
    {
        // Move to next patrol point
        BehaviorMemory.CurrentPatrolIndex = (BehaviorMemory.CurrentPatrolIndex + 1) % BehaviorMemory.PatrolPoints.Num();
    }
    else
    {
        // Move towards current patrol point
        FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * MovementSpeed * GetWorld()->GetDeltaSeconds();
        Owner->SetActorLocation(NewLocation);
    }
}

void UNPC_BehaviorComponent::ReactToPlayer()
{
    AActor* Player = GetPlayerActor();
    if (!Player)
    {
        return;
    }

    UpdatePlayerMemory(Player->GetActorLocation());
    
    float Distance = GetDistanceToPlayer();
    
    // Decide reaction based on distance and personality
    if (Distance < FleeRadius && Courage < 0.3f)
    {
        StartFleeing();
    }
    else if (Distance < DetectionRadius && Curiosity > 0.5f)
    {
        StartInvestigating(Player->GetActorLocation());
    }
}

void UNPC_BehaviorComponent::UpdatePlayerMemory(const FVector& PlayerLocation)
{
    BehaviorMemory.LastKnownPlayerLocation = PlayerLocation;
    BehaviorMemory.TimeSincePlayerSeen = 0.0f;
}

void UNPC_BehaviorComponent::StartFleeing()
{
    SetBehaviorState(ENPC_BehaviorState::Fleeing);
    
    // TODO: Implement flee behavior - move away from player
    AActor* Player = GetPlayerActor();
    AActor* Owner = GetOwner();
    
    if (Player && Owner)
    {
        FVector FleeDirection = (Owner->GetActorLocation() - Player->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = Owner->GetActorLocation() + FleeDirection * FleeRadius;
        
        // Simple flee movement
        Owner->SetActorLocation(FMath::VInterpTo(
            Owner->GetActorLocation(), 
            FleeTarget, 
            GetWorld()->GetDeltaSeconds(), 
            2.0f
        ));
    }
}

void UNPC_BehaviorComponent::StartInvestigating(const FVector& Location)
{
    SetBehaviorState(ENPC_BehaviorState::Investigating);
    BehaviorMemory.LastKnownPlayerLocation = Location;
}

void UNPC_BehaviorComponent::HandleIdleState(float DeltaTime)
{
    // Randomly decide to start patrolling
    static float IdleTimer = 0.0f;
    IdleTimer += DeltaTime;
    
    if (IdleTimer > 3.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        IdleTimer = 0.0f;
    }
}

void UNPC_BehaviorComponent::HandlePatrollingState(float DeltaTime)
{
    UpdatePatrol();
    
    // Occasionally return to idle
    static float PatrolTimer = 0.0f;
    PatrolTimer += DeltaTime;
    
    if (PatrolTimer > 15.0f && FMath::RandRange(0.0f, 1.0f) < 0.1f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        PatrolTimer = 0.0f;
    }
}

void UNPC_BehaviorComponent::HandleInvestigatingState(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    // Move towards last known player location
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetLocation = BehaviorMemory.LastKnownPlayerLocation;
    
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    if (Distance < 300.0f || BehaviorMemory.TimeSincePlayerSeen > 10.0f)
    {
        // Finished investigating or lost interest
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
    else
    {
        // Move towards investigation point
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * MovementSpeed * 0.5f * DeltaTime;
        Owner->SetActorLocation(NewLocation);
    }
}

void UNPC_BehaviorComponent::HandleFleeingState(float DeltaTime)
{
    // Return to patrol after fleeing for a while
    if (BehaviorMemory.TimeSincePlayerSeen > 8.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

AActor* UNPC_BehaviorComponent::GetPlayerActor()
{
    return UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
}

float UNPC_BehaviorComponent::GetDistanceToPlayer()
{
    AActor* Player = GetPlayerActor();
    if (!Player || !GetOwner())
    {
        return FLT_MAX;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}

bool UNPC_BehaviorComponent::IsPlayerInRange(float Range)
{
    return GetDistanceToPlayer() <= Range;
}

FVector UNPC_BehaviorComponent::GetRandomPatrolPoint()
{
    if (!GetOwner())
    {
        return FVector::ZeroVector;
    }
    
    FVector BaseLocation = GetOwner()->GetActorLocation();
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    
    return BaseLocation + FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
}