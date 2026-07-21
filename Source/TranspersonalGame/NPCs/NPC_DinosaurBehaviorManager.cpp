#include "NPC_DinosaurBehaviorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"
#include "AIController.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default behavior data
    BehaviorData.Species = ENPC_DinosaurSpecies::TRex;
    BehaviorData.PatrolRadius = 5000.0f;
    BehaviorData.DetectionRange = 3000.0f;
    BehaviorData.AttackRange = 300.0f;
    BehaviorData.MovementSpeed = 600.0f;
    BehaviorData.Aggression = 0.7f;
    BehaviorData.bIsPackHunter = false;
    BehaviorData.HungerLevel = 50.0f;
    BehaviorData.FearLevel = 0.0f;
    
    CurrentState = ENPC_DinosaurState::Idle;
    CurrentTarget = nullptr;
    StateChangeTimer = 0.0f;
    IdleTime = 3.0f;
    PatrolTime = 10.0f;
}

void UNPC_DinosaurBehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set patrol center to current location
    if (GetOwner())
    {
        PatrolCenter = GetOwner()->GetActorLocation();
        CurrentPatrolTarget = PatrolCenter;
        
        // Initialize species-specific data
        InitializeSpeciesData();
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorManager: Initialized for %s at %s"), 
               *GetOwner()->GetName(), 
               *PatrolCenter.ToString());
    }
}

void UNPC_DinosaurBehaviorManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner() || !GetWorld())
    {
        return;
    }
    
    StateChangeTimer += DeltaTime;
    
    // Update behavior based on current state
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            UpdateIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            UpdatePatrollingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunting:
            UpdateHuntingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Chasing:
            UpdateChasingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Attacking:
            UpdateAttackingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Fleeing:
            UpdateFleeingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Feeding:
            UpdateFeedingState(DeltaTime);
            break;
    }
}

void UNPC_DinosaurBehaviorManager::SetDinosaurSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    BehaviorData.Species = NewSpecies;
    InitializeSpeciesData();
}

void UNPC_DinosaurBehaviorManager::ChangeState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorManager: %s changed state from %d to %d"), 
               *GetOwner()->GetName(), 
               (int32)OldState, 
               (int32)NewState);
    }
}

AActor* UNPC_DinosaurBehaviorManager::FindNearestPlayer()
{
    if (!GetWorld())
    {
        return nullptr;
    }
    
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (PC && PC->GetPawn())
    {
        return PC->GetPawn();
    }
    
    return nullptr;
}

bool UNPC_DinosaurBehaviorManager::IsPlayerInRange(float Range)
{
    AActor* Player = FindNearestPlayer();
    if (!Player || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    return Distance <= Range;
}

void UNPC_DinosaurBehaviorManager::StartPatrol()
{
    ChangeState(ENPC_DinosaurState::Patrolling);
    CurrentPatrolTarget = GetRandomPatrolPoint();
    MoveToLocation(CurrentPatrolTarget);
}

void UNPC_DinosaurBehaviorManager::StartHunting(AActor* Target)
{
    CurrentTarget = Target;
    ChangeState(ENPC_DinosaurState::Hunting);
}

void UNPC_DinosaurBehaviorManager::StartAttacking(AActor* Target)
{
    CurrentTarget = Target;
    ChangeState(ENPC_DinosaurState::Attacking);
}

void UNPC_DinosaurBehaviorManager::StartFleeing()
{
    ChangeState(ENPC_DinosaurState::Fleeing);
    CurrentTarget = nullptr;
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint()
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(BehaviorData.PatrolRadius * 0.3f, BehaviorData.PatrolRadius);
    
    FVector Offset = FVector(
        FMath::Cos(RandomAngle) * RandomDistance,
        FMath::Sin(RandomAngle) * RandomDistance,
        0.0f
    );
    
    return PatrolCenter + Offset;
}

void UNPC_DinosaurBehaviorManager::MoveToLocation(FVector TargetLocation)
{
    if (!GetOwner())
    {
        return;
    }
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (OwnerPawn && OwnerPawn->GetController())
    {
        AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController());
        if (AIController)
        {
            // Use AI movement if available
            AIController->MoveToLocation(TargetLocation, 100.0f);
        }
        else
        {
            // Simple movement towards target
            FVector CurrentLocation = GetOwner()->GetActorLocation();
            FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
            FVector NewLocation = CurrentLocation + (Direction * BehaviorData.MovementSpeed * GetWorld()->GetDeltaSeconds());
            GetOwner()->SetActorLocation(NewLocation);
        }
    }
}

void UNPC_DinosaurBehaviorManager::UpdateIdleState(float DeltaTime)
{
    // Check for nearby players
    if (IsPlayerInRange(BehaviorData.DetectionRange))
    {
        AActor* Player = FindNearestPlayer();
        if (Player)
        {
            StartHunting(Player);
            return;
        }
    }
    
    // Transition to patrol after idle time
    if (StateChangeTimer >= IdleTime)
    {
        StartPatrol();
    }
}

void UNPC_DinosaurBehaviorManager::UpdatePatrollingState(float DeltaTime)
{
    // Check for nearby players
    if (IsPlayerInRange(BehaviorData.DetectionRange))
    {
        AActor* Player = FindNearestPlayer();
        if (Player)
        {
            StartHunting(Player);
            return;
        }
    }
    
    // Check if reached patrol target
    if (HasReachedTarget(CurrentPatrolTarget))
    {
        ChangeState(ENPC_DinosaurState::Idle);
    }
    else
    {
        MoveToLocation(CurrentPatrolTarget);
    }
    
    // Get new patrol point after patrol time
    if (StateChangeTimer >= PatrolTime)
    {
        CurrentPatrolTarget = GetRandomPatrolPoint();
        StateChangeTimer = 0.0f;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateHuntingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        ChangeState(ENPC_DinosaurState::Idle);
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Start attacking if close enough
    if (DistanceToTarget <= BehaviorData.AttackRange)
    {
        StartAttacking(CurrentTarget);
    }
    // Continue chasing
    else if (DistanceToTarget <= BehaviorData.DetectionRange * 1.5f)
    {
        ChangeState(ENPC_DinosaurState::Chasing);
        MoveToLocation(CurrentTarget->GetActorLocation());
    }
    // Lost target, return to patrol
    else
    {
        CurrentTarget = nullptr;
        ChangeState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateChasingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        ChangeState(ENPC_DinosaurState::Idle);
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Start attacking if close enough
    if (DistanceToTarget <= BehaviorData.AttackRange)
    {
        StartAttacking(CurrentTarget);
    }
    // Continue chasing
    else if (DistanceToTarget <= BehaviorData.DetectionRange * 1.5f)
    {
        MoveToLocation(CurrentTarget->GetActorLocation());
    }
    // Lost target, return to patrol
    else
    {
        CurrentTarget = nullptr;
        ChangeState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateAttackingState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        ChangeState(ENPC_DinosaurState::Idle);
        return;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Continue attacking if in range
    if (DistanceToTarget <= BehaviorData.AttackRange * 1.2f)
    {
        // Perform attack logic here
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorManager: %s attacking %s"), 
               *GetOwner()->GetName(), 
               *CurrentTarget->GetName());
        
        // Attack for 2 seconds then chase again
        if (StateChangeTimer >= 2.0f)
        {
            ChangeState(ENPC_DinosaurState::Chasing);
        }
    }
    else
    {
        // Target moved away, chase again
        ChangeState(ENPC_DinosaurState::Chasing);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateFleeingState(float DeltaTime)
{
    // Simple fleeing logic - move away from patrol center
    FVector FleeDirection = (GetOwner()->GetActorLocation() - PatrolCenter).GetSafeNormal();
    FVector FleeTarget = GetOwner()->GetActorLocation() + (FleeDirection * 2000.0f);
    MoveToLocation(FleeTarget);
    
    // Return to idle after fleeing for 5 seconds
    if (StateChangeTimer >= 5.0f)
    {
        ChangeState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorManager::UpdateFeedingState(float DeltaTime)
{
    // Feeding behavior - stay in place for a while
    if (StateChangeTimer >= 10.0f)
    {
        BehaviorData.HungerLevel = FMath::Min(100.0f, BehaviorData.HungerLevel + 30.0f);
        ChangeState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorManager::InitializeSpeciesData()
{
    switch (BehaviorData.Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            BehaviorData.PatrolRadius = 5000.0f;
            BehaviorData.DetectionRange = 3000.0f;
            BehaviorData.AttackRange = 300.0f;
            BehaviorData.MovementSpeed = 600.0f;
            BehaviorData.Aggression = 0.9f;
            BehaviorData.bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            BehaviorData.PatrolRadius = 3000.0f;
            BehaviorData.DetectionRange = 2000.0f;
            BehaviorData.AttackRange = 200.0f;
            BehaviorData.MovementSpeed = 800.0f;
            BehaviorData.Aggression = 0.8f;
            BehaviorData.bIsPackHunter = true;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            BehaviorData.PatrolRadius = 4000.0f;
            BehaviorData.DetectionRange = 1500.0f;
            BehaviorData.AttackRange = 400.0f;
            BehaviorData.MovementSpeed = 400.0f;
            BehaviorData.Aggression = 0.3f;
            BehaviorData.bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            BehaviorData.PatrolRadius = 6000.0f;
            BehaviorData.DetectionRange = 1000.0f;
            BehaviorData.AttackRange = 500.0f;
            BehaviorData.MovementSpeed = 200.0f;
            BehaviorData.Aggression = 0.1f;
            BehaviorData.bIsPackHunter = false;
            break;
            
        case ENPC_DinosaurSpecies::Ankylosaurus:
            BehaviorData.PatrolRadius = 3500.0f;
            BehaviorData.DetectionRange = 1200.0f;
            BehaviorData.AttackRange = 350.0f;
            BehaviorData.MovementSpeed = 300.0f;
            BehaviorData.Aggression = 0.4f;
            BehaviorData.bIsPackHunter = false;
            break;
    }
}

bool UNPC_DinosaurBehaviorManager::HasReachedTarget(FVector TargetLocation, float Tolerance)
{
    if (!GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), TargetLocation);
    return Distance <= Tolerance;
}

float UNPC_DinosaurBehaviorManager::GetDistanceToPlayer()
{
    AActor* Player = FindNearestPlayer();
    if (!Player || !GetOwner())
    {
        return -1.0f;
    }
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
}