#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize behavior system
    SetBehaviorState(ENPC_BehaviorState::Idle);
    
    // Start behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(BehaviorTimer, this, &UNPCBehaviorComponent::UpdateBehavior, 0.5f, true);
    }
    
    // Setup default patrol if no points specified
    if (PatrolPoints.Num() == 0)
    {
        FNPC_PatrolPoint Point1;
        Point1.Location = GetOwner()->GetActorLocation() + FVector(500, 0, 0);
        Point1.WaitTime = 2.0f;
        
        FNPC_PatrolPoint Point2;
        Point2.Location = GetOwner()->GetActorLocation() + FVector(-500, 0, 0);
        Point2.WaitTime = 2.0f;
        
        PatrolPoints.Add(Point1);
        PatrolPoints.Add(Point2);
    }
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    StateTimer += DeltaTime;
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;
        
        // Log state change for debugging
        if (GEngine)
        {
            FString StateName = UEnum::GetValueAsString(CurrentState);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, 
                FString::Printf(TEXT("NPC %s: State changed to %s"), 
                *GetOwner()->GetName(), *StateName));
        }
    }
}

void UNPCBehaviorComponent::StartPatrolling()
{
    if (PatrolPoints.Num() > 0)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
        CurrentPatrolIndex = 0;
        MoveToNextPatrolPoint();
    }
}

void UNPCBehaviorComponent::InvestigateLocation(FVector Location)
{
    TargetLocation = Location;
    bHasValidTarget = true;
    SetBehaviorState(ENPC_BehaviorState::Investigating);
}

void UNPCBehaviorComponent::FleeFromThreat(FVector ThreatLocation)
{
    // Calculate flee direction (opposite from threat)
    FVector FleeDirection = (GetOwner()->GetActorLocation() - ThreatLocation).GetSafeNormal();
    TargetLocation = GetOwner()->GetActorLocation() + (FleeDirection * FleeRadius * 2.0f);
    bHasValidTarget = true;
    SetBehaviorState(ENPC_BehaviorState::Fleeing);
    
    // Clear memory after fleeing
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(MemoryTimer, this, &UNPCBehaviorComponent::ClearPlayerMemory, MemoryDuration, false);
    }
}

bool UNPCBehaviorComponent::CanSeePlayer()
{
    APawn* Player = GetNearestPlayer();
    if (!Player)
    {
        return false;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector PlayerLocation = Player->GetActorLocation();
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
    
    if (Distance > DetectionRadius)
    {
        return false;
    }
    
    // Simple line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLocation,
        PlayerLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstruction
}

APawn* UNPCBehaviorComponent::GetNearestPlayer()
{
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

void UNPCBehaviorComponent::UpdateBehavior()
{
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            ProcessIdleState();
            break;
        case ENPC_BehaviorState::Patrolling:
            ProcessPatrollingState();
            break;
        case ENPC_BehaviorState::Investigating:
            ProcessInvestigatingState();
            break;
        case ENPC_BehaviorState::Fleeing:
            ProcessFleeingState();
            break;
        default:
            break;
    }
}

void UNPCBehaviorComponent::ProcessIdleState()
{
    // Check for player
    if (CanSeePlayer())
    {
        APawn* Player = GetNearestPlayer();
        if (Player)
        {
            LastKnownPlayerLocation = Player->GetActorLocation();
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
            
            if (Distance < FleeRadius)
            {
                FleeFromThreat(LastKnownPlayerLocation);
            }
            else
            {
                InvestigateLocation(LastKnownPlayerLocation);
            }
        }
    }
    else if (StateTimer > 3.0f) // Idle for 3 seconds, start patrolling
    {
        StartPatrolling();
    }
}

void UNPCBehaviorComponent::ProcessPatrollingState()
{
    // Check for player first
    if (CanSeePlayer())
    {
        APawn* Player = GetNearestPlayer();
        if (Player)
        {
            LastKnownPlayerLocation = Player->GetActorLocation();
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
            
            if (Distance < FleeRadius)
            {
                FleeFromThreat(LastKnownPlayerLocation);
                return;
            }
        }
    }
    
    // Continue patrol
    if (PatrolPoints.Num() > 0)
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector PatrolTarget = PatrolPoints[CurrentPatrolIndex].Location;
        float Distance = FVector::Dist(CurrentLocation, PatrolTarget);
        
        if (Distance < 100.0f) // Reached patrol point
        {
            if (StateTimer > PatrolPoints[CurrentPatrolIndex].WaitTime)
            {
                MoveToNextPatrolPoint();
            }
        }
        else
        {
            // Move towards patrol point (simplified movement)
            FVector Direction = (PatrolTarget - CurrentLocation).GetSafeNormal();
            FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * 0.5f * GetWorld()->GetDeltaSeconds());
            GetOwner()->SetActorLocation(NewLocation);
        }
    }
}

void UNPCBehaviorComponent::ProcessInvestigatingState()
{
    if (!bHasValidTarget)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    if (Distance < 150.0f || StateTimer > 8.0f) // Reached target or timeout
    {
        bHasValidTarget = false;
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
    else
    {
        // Move towards investigation target
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * 0.8f * GetWorld()->GetDeltaSeconds());
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UNPCBehaviorComponent::ProcessFleeingState()
{
    if (!bHasValidTarget)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
        return;
    }
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist(CurrentLocation, TargetLocation);
    
    if (Distance < 200.0f || StateTimer > 5.0f) // Reached safe distance or timeout
    {
        bHasValidTarget = false;
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
    else
    {
        // Move away from threat
        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * 1.5f * GetWorld()->GetDeltaSeconds());
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UNPCBehaviorComponent::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return;
    }
    
    CurrentPatrolIndex++;
    if (CurrentPatrolIndex >= PatrolPoints.Num())
    {
        if (bLoopPatrol)
        {
            CurrentPatrolIndex = 0;
        }
        else
        {
            CurrentPatrolIndex = PatrolPoints.Num() - 1;
            SetBehaviorState(ENPC_BehaviorState::Idle);
            return;
        }
    }
    
    StateTimer = 0.0f;
}

void UNPCBehaviorComponent::ClearPlayerMemory()
{
    LastKnownPlayerLocation = FVector::ZeroVector;
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, 
            FString::Printf(TEXT("NPC %s: Memory cleared"), *GetOwner()->GetName()));
    }
}