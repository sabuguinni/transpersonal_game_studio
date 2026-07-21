#include "NPC_DinosaurBehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default values
    TimeSinceLastUpdate = 0.0f;
    LastKnownPlayerLocation = FVector::ZeroVector;
    bPlayerDetected = false;
    PatrolWaitTime = 0.0f;
    
    // Initialize behavior data with defaults
    BehaviorData = FNPC_DinosaurBehaviorData();
    
    AIControllerRef = nullptr;
    BehaviorTreeAsset = nullptr;
    BlackboardAsset = nullptr;
}

void UNPC_DinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    // Get AI Controller reference
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        AIControllerRef = Cast<AAIController>(OwnerPawn->GetController());
    }
    
    // Set home location to current location
    if (GetOwner())
    {
        BehaviorData.HomeLocation = GetOwner()->GetActorLocation();
        BehaviorData.PatrolTarget = BehaviorData.HomeLocation;
    }
    
    // Configure species-specific behavior
    ConfigureSpeciesBehavior();
    
    // Initialize blackboard values
    UpdateBlackboardValues();
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree initialized for %s"), 
           *GetOwner()->GetName());
}

void UNPC_DinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    TimeSinceLastUpdate += DeltaTime;
    BehaviorData.StateTimer += DeltaTime;
    
    // Update behavior logic
    UpdateBehaviorLogic(DeltaTime);
    
    // Update blackboard values every 0.5 seconds
    if (TimeSinceLastUpdate >= 0.5f)
    {
        UpdateBlackboardValues();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UNPC_DinosaurBehaviorTree::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (BehaviorData.CurrentState != NewState)
    {
        TransitionToState(NewState);
    }
}

ENPC_DinosaurState UNPC_DinosaurBehaviorTree::GetDinosaurState() const
{
    return BehaviorData.CurrentState;
}

void UNPC_DinosaurBehaviorTree::InitializeBehavior(ENPC_DinosaurSpecies InSpecies, FVector InHomeLocation)
{
    BehaviorData.Species = InSpecies;
    BehaviorData.HomeLocation = InHomeLocation;
    BehaviorData.PatrolTarget = InHomeLocation;
    
    ConfigureSpeciesBehavior();
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur behavior initialized: Species=%d, Home=(%f,%f,%f)"),
           (int32)InSpecies, InHomeLocation.X, InHomeLocation.Y, InHomeLocation.Z);
}

void UNPC_DinosaurBehaviorTree::UpdateBehaviorLogic(float DeltaTime)
{
    // Update hunger over time
    BehaviorData.Hunger = FMath::Max(0.0f, BehaviorData.Hunger - (DeltaTime * 0.5f));
    
    // Find nearest player
    FindNearestPlayer();
    
    // Execute state-specific logic
    switch (BehaviorData.CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            HandleIdleState(DeltaTime);
            break;
        case ENPC_DinosaurState::Patrolling:
            HandlePatrollingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Chasing:
            HandleChasingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Fleeing:
            HandleFleeingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Feeding:
            HandleFeedingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Resting:
            HandleRestingState(DeltaTime);
            break;
        case ENPC_DinosaurState::Territorial:
            HandleTerritorialState(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorTree::FindNearestPlayer()
{
    if (!GetWorld()) return;
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        APawn* PlayerPawn = PlayerController->GetPawn();
        FVector PlayerLocation = PlayerPawn->GetActorLocation();
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerLocation);
        
        BehaviorData.TargetActor = PlayerPawn;
        LastKnownPlayerLocation = PlayerLocation;
        bPlayerDetected = true;
        
        // Update blackboard with player information
        SetBlackboardVector("PlayerLocation", PlayerLocation);
        SetBlackboardFloat("DistanceToPlayer", DistanceToPlayer);
        SetBlackboardObject("PlayerActor", PlayerPawn);
        SetBlackboardBool("PlayerDetected", true);
    }
    else
    {
        bPlayerDetected = false;
        SetBlackboardBool("PlayerDetected", false);
    }
}

void UNPC_DinosaurBehaviorTree::GeneratePatrolPoint()
{
    // Generate random point within patrol radius
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(BehaviorData.PatrolRadius * 0.3f, BehaviorData.PatrolRadius);
    FVector NewPatrolPoint = BehaviorData.HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find valid navigation point
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(NewPatrolPoint, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            BehaviorData.PatrolTarget = NavLocation.Location;
        }
        else
        {
            BehaviorData.PatrolTarget = NewPatrolPoint;
        }
    }
    else
    {
        BehaviorData.PatrolTarget = NewPatrolPoint;
    }
    
    SetBlackboardVector("PatrolTarget", BehaviorData.PatrolTarget);
}

bool UNPC_DinosaurBehaviorTree::IsPlayerInRange(float Range) const
{
    if (!bPlayerDetected || !GetOwner()) return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
    return Distance <= Range;
}

float UNPC_DinosaurBehaviorTree::GetDistanceToPlayer() const
{
    if (!bPlayerDetected || !GetOwner()) return -1.0f;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), LastKnownPlayerLocation);
}

void UNPC_DinosaurBehaviorTree::MoveToLocation(FVector TargetLocation)
{
    if (AIControllerRef)
    {
        AIControllerRef->MoveToLocation(TargetLocation, 100.0f);
    }
}

void UNPC_DinosaurBehaviorTree::AttackTarget()
{
    if (BehaviorData.TargetActor && IsPlayerInRange(BehaviorData.AttackRadius))
    {
        // Implement attack logic here
        UE_LOG(LogTemp, Warning, TEXT("%s attacking target!"), *GetOwner()->GetName());
        
        // Deal damage to target (placeholder)
        // In a real implementation, this would interface with a damage system
        
        SetBlackboardBool("IsAttacking", true);
    }
}

void UNPC_DinosaurBehaviorTree::ConfigureSpeciesBehavior()
{
    switch (BehaviorData.Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            BehaviorData.PatrolRadius = 8000.0f;
            BehaviorData.ChaseRadius = 5000.0f;
            BehaviorData.AttackRadius = 400.0f;
            BehaviorData.MovementSpeed = 600.0f;
            BehaviorData.Aggression = 0.9f;
            BehaviorData.Health = 1000.0f;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            BehaviorData.PatrolRadius = 3000.0f;
            BehaviorData.ChaseRadius = 2000.0f;
            BehaviorData.AttackRadius = 200.0f;
            BehaviorData.MovementSpeed = 800.0f;
            BehaviorData.Aggression = 0.8f;
            BehaviorData.Health = 300.0f;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            BehaviorData.PatrolRadius = 4000.0f;
            BehaviorData.ChaseRadius = 1500.0f;
            BehaviorData.AttackRadius = 300.0f;
            BehaviorData.MovementSpeed = 400.0f;
            BehaviorData.Aggression = 0.3f;
            BehaviorData.Health = 800.0f;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            BehaviorData.PatrolRadius = 6000.0f;
            BehaviorData.ChaseRadius = 1000.0f;
            BehaviorData.AttackRadius = 500.0f;
            BehaviorData.MovementSpeed = 200.0f;
            BehaviorData.Aggression = 0.1f;
            BehaviorData.Health = 1500.0f;
            break;
            
        case ENPC_DinosaurSpecies::Ankylosaurus:
            BehaviorData.PatrolRadius = 2000.0f;
            BehaviorData.ChaseRadius = 800.0f;
            BehaviorData.AttackRadius = 250.0f;
            BehaviorData.MovementSpeed = 300.0f;
            BehaviorData.Aggression = 0.2f;
            BehaviorData.Health = 1200.0f;
            break;
            
        case ENPC_DinosaurSpecies::Parasaurolophus:
            BehaviorData.PatrolRadius = 5000.0f;
            BehaviorData.ChaseRadius = 1200.0f;
            BehaviorData.AttackRadius = 150.0f;
            BehaviorData.MovementSpeed = 500.0f;
            BehaviorData.Aggression = 0.1f;
            BehaviorData.Health = 600.0f;
            break;
    }
}

void UNPC_DinosaurBehaviorTree::UpdateBlackboardValues()
{
    if (!AIControllerRef || !AIControllerRef->GetBlackboardComponent()) return;
    
    UBlackboardComponent* BlackboardComp = AIControllerRef->GetBlackboardComponent();
    
    // Update basic state information
    SetBlackboardFloat("Health", BehaviorData.Health);
    SetBlackboardFloat("Hunger", BehaviorData.Hunger);
    SetBlackboardFloat("Aggression", BehaviorData.Aggression);
    SetBlackboardFloat("MovementSpeed", BehaviorData.MovementSpeed);
    SetBlackboardVector("HomeLocation", BehaviorData.HomeLocation);
    SetBlackboardVector("PatrolTarget", BehaviorData.PatrolTarget);
    
    // Update state enum (as integer)
    BlackboardComp->SetValueAsInt("DinosaurState", (int32)BehaviorData.CurrentState);
    BlackboardComp->SetValueAsInt("DinosaurSpecies", (int32)BehaviorData.Species);
}

void UNPC_DinosaurBehaviorTree::SetBlackboardVector(const FString& KeyName, FVector Value)
{
    if (AIControllerRef && AIControllerRef->GetBlackboardComponent())
    {
        AIControllerRef->GetBlackboardComponent()->SetValueAsVector(*KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardFloat(const FString& KeyName, float Value)
{
    if (AIControllerRef && AIControllerRef->GetBlackboardComponent())
    {
        AIControllerRef->GetBlackboardComponent()->SetValueAsFloat(*KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardBool(const FString& KeyName, bool Value)
{
    if (AIControllerRef && AIControllerRef->GetBlackboardComponent())
    {
        AIControllerRef->GetBlackboardComponent()->SetValueAsBool(*KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardObject(const FString& KeyName, UObject* Value)
{
    if (AIControllerRef && AIControllerRef->GetBlackboardComponent())
    {
        AIControllerRef->GetBlackboardComponent()->SetValueAsObject(*KeyName, Value);
    }
}

// State handling functions
void UNPC_DinosaurBehaviorTree::HandleIdleState(float DeltaTime)
{
    // Check if player is nearby
    if (IsPlayerInRange(BehaviorData.ChaseRadius) && BehaviorData.Aggression > 0.3f)
    {
        TransitionToState(ENPC_DinosaurState::Chasing);
        return;
    }
    
    // Check if hungry
    if (BehaviorData.Hunger < 30.0f)
    {
        TransitionToState(ENPC_DinosaurState::Hunting);
        return;
    }
    
    // Start patrolling after idle time
    if (BehaviorData.StateTimer > 3.0f)
    {
        TransitionToState(ENPC_DinosaurState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorTree::HandlePatrollingState(float DeltaTime)
{
    // Check if player is nearby
    if (IsPlayerInRange(BehaviorData.ChaseRadius) && BehaviorData.Aggression > 0.3f)
    {
        TransitionToState(ENPC_DinosaurState::Chasing);
        return;
    }
    
    // Move to patrol target
    if (GetOwner())
    {
        float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), BehaviorData.PatrolTarget);
        
        if (DistanceToTarget < 200.0f)
        {
            // Reached patrol point, generate new one
            GeneratePatrolPoint();
            TransitionToState(ENPC_DinosaurState::Idle);
        }
        else
        {
            MoveToLocation(BehaviorData.PatrolTarget);
        }
    }
}

void UNPC_DinosaurBehaviorTree::HandleHuntingState(float DeltaTime)
{
    // Look for food sources or smaller prey
    // For now, just wander around looking for food
    
    if (BehaviorData.StateTimer > 10.0f)
    {
        // Give up hunting, go back to patrolling
        TransitionToState(ENPC_DinosaurState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorTree::HandleChasingState(float DeltaTime)
{
    if (!IsPlayerInRange(BehaviorData.ChaseRadius * 1.5f))
    {
        // Lost target, go back to patrolling
        TransitionToState(ENPC_DinosaurState::Patrolling);
        return;
    }
    
    if (IsPlayerInRange(BehaviorData.AttackRadius))
    {
        TransitionToState(ENPC_DinosaurState::Attacking);
        return;
    }
    
    // Chase the player
    MoveToLocation(LastKnownPlayerLocation);
}

void UNPC_DinosaurBehaviorTree::HandleAttackingState(float DeltaTime)
{
    if (!IsPlayerInRange(BehaviorData.AttackRadius * 1.2f))
    {
        // Target moved away, chase again
        TransitionToState(ENPC_DinosaurState::Chasing);
        return;
    }
    
    // Perform attack
    AttackTarget();
    
    // Attack duration
    if (BehaviorData.StateTimer > 2.0f)
    {
        TransitionToState(ENPC_DinosaurState::Chasing);
    }
}

void UNPC_DinosaurBehaviorTree::HandleFleeingState(float DeltaTime)
{
    // Run away from threat
    if (BehaviorData.StateTimer > 5.0f)
    {
        TransitionToState(ENPC_DinosaurState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorTree::HandleFeedingState(float DeltaTime)
{
    // Restore hunger
    BehaviorData.Hunger = FMath::Min(100.0f, BehaviorData.Hunger + (DeltaTime * 10.0f));
    
    if (BehaviorData.Hunger > 80.0f || BehaviorData.StateTimer > 8.0f)
    {
        TransitionToState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorTree::HandleRestingState(float DeltaTime)
{
    // Restore health slowly
    BehaviorData.Health = FMath::Min(100.0f, BehaviorData.Health + (DeltaTime * 2.0f));
    
    if (BehaviorData.StateTimer > 10.0f)
    {
        TransitionToState(ENPC_DinosaurState::Idle);
    }
}

void UNPC_DinosaurBehaviorTree::HandleTerritorialState(float DeltaTime)
{
    // Defend territory aggressively
    if (IsPlayerInRange(BehaviorData.ChaseRadius))
    {
        TransitionToState(ENPC_DinosaurState::Chasing);
    }
    else if (BehaviorData.StateTimer > 15.0f)
    {
        TransitionToState(ENPC_DinosaurState::Patrolling);
    }
}

void UNPC_DinosaurBehaviorTree::TransitionToState(ENPC_DinosaurState NewState)
{
    if (CanTransitionToState(NewState))
    {
        OnStateExit(BehaviorData.CurrentState);
        ENPC_DinosaurState OldState = BehaviorData.CurrentState;
        BehaviorData.CurrentState = NewState;
        BehaviorData.LastStateChangeTime = GetWorld()->GetTimeSeconds();
        BehaviorData.StateTimer = 0.0f;
        OnStateEnter(NewState);
        
        UE_LOG(LogTemp, Log, TEXT("%s: State transition %d -> %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);
    }
}

bool UNPC_DinosaurBehaviorTree::CanTransitionToState(ENPC_DinosaurState TargetState) const
{
    // Add transition rules here if needed
    return true;
}

void UNPC_DinosaurBehaviorTree::OnStateEnter(ENPC_DinosaurState EnteredState)
{
    switch (EnteredState)
    {
        case ENPC_DinosaurState::Patrolling:
            GeneratePatrolPoint();
            break;
        case ENPC_DinosaurState::Attacking:
            SetBlackboardBool("IsAttacking", true);
            break;
        default:
            break;
    }
}

void UNPC_DinosaurBehaviorTree::OnStateExit(ENPC_DinosaurState ExitedState)
{
    switch (ExitedState)
    {
        case ENPC_DinosaurState::Attacking:
            SetBlackboardBool("IsAttacking", false);
            break;
        default:
            break;
    }
}