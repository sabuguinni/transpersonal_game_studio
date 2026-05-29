#include "NPC_DinosaurBehaviorTree.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    CurrentState = ENPC_DinosaurState::Idle;
    SightRange = 3000.0f;
    HearingRange = 1500.0f;
    TerritoryRadius = 5000.0f;
    PatrolSpeed = 200.0f;
    ChaseSpeed = 600.0f;
    AttackRange = 300.0f;
    HomeLocation = FVector::ZeroVector;
    StateChangeTime = 0.0f;

    // Initialize memory
    DinosaurMemory = FNPC_DinosaurMemory();
}

void UNPC_DinosaurBehaviorTree::InitializeBehaviorTree(AAIController* AIController)
{
    if (!AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorTree: AIController is null"));
        return;
    }

    UBehaviorTreeComponent* BTComponent = AIController->GetBehaviorTreeComponent();
    UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent();

    if (!BTComponent || !BlackboardComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorTree: Missing BT or BB component"));
        return;
    }

    // Set home location to current pawn location
    if (APawn* ControlledPawn = AIController->GetPawn())
    {
        HomeLocation = ControlledPawn->GetActorLocation();
        DinosaurMemory.bIsInTerritory = true;
        
        // Initialize patrol points around home location
        AddPatrolPoint(HomeLocation + FVector(1000, 0, 0));
        AddPatrolPoint(HomeLocation + FVector(0, 1000, 0));
        AddPatrolPoint(HomeLocation + FVector(-1000, 0, 0));
        AddPatrolPoint(HomeLocation + FVector(0, -1000, 0));
    }

    // Initialize blackboard values
    BlackboardComp->SetValueAsEnum(FName("DinosaurState"), static_cast<uint8>(CurrentState));
    BlackboardComp->SetValueAsVector(FName("HomeLocation"), HomeLocation);
    BlackboardComp->SetValueAsFloat(FName("SightRange"), SightRange);
    BlackboardComp->SetValueAsFloat(FName("TerritoryRadius"), TerritoryRadius);
    BlackboardComp->SetValueAsFloat(FName("PatrolSpeed"), PatrolSpeed);
    BlackboardComp->SetValueAsFloat(FName("ChaseSpeed"), ChaseSpeed);
    BlackboardComp->SetValueAsFloat(FName("AttackRange"), AttackRange);

    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: Initialized for %s"), 
           AIController->GetPawn() ? *AIController->GetPawn()->GetName() : TEXT("Unknown"));
}

void UNPC_DinosaurBehaviorTree::UpdateDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState OldState = CurrentState;
        CurrentState = NewState;
        StateChangeTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        
        OnStateChanged(OldState, NewState);
        
        UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: State changed from %d to %d"), 
               static_cast<int32>(OldState), static_cast<int32>(NewState));
    }
}

bool UNPC_DinosaurBehaviorTree::CanSeePlayer(APawn* Player)
{
    if (!Player || !GetWorld())
    {
        return false;
    }

    // Get AI controller's pawn
    AAIController* AIController = nullptr;
    if (UWorld* World = GetWorld())
    {
        for (FConstPawnIterator Iterator = World->GetPawnIterator(); Iterator; ++Iterator)
        {
            if (APawn* Pawn = Iterator->Get())
            {
                if (AAIController* AI = Cast<AAIController>(Pawn->GetController()))
                {
                    AIController = AI;
                    break;
                }
            }
        }
    }

    if (!AIController || !AIController->GetPawn())
    {
        return false;
    }

    FVector AILocation = AIController->GetPawn()->GetActorLocation();
    FVector PlayerLocation = Player->GetActorLocation();
    float Distance = FVector::Dist(AILocation, PlayerLocation);

    // Check if player is within sight range
    if (Distance > SightRange)
    {
        return false;
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(AIController->GetPawn());
    QueryParams.AddIgnoredActor(Player);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        AILocation,
        PlayerLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    // If no hit, we have line of sight
    bool bCanSee = !bHit;
    
    if (bCanSee)
    {
        UpdatePlayerMemory(PlayerLocation);
    }

    return bCanSee;
}

bool UNPC_DinosaurBehaviorTree::IsInTerritory(FVector Location)
{
    float DistanceFromHome = FVector::Dist(Location, HomeLocation);
    bool bInTerritory = DistanceFromHome <= TerritoryRadius;
    
    DinosaurMemory.bIsInTerritory = bInTerritory;
    
    return bInTerritory;
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint()
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
    {
        return HomeLocation;
    }

    FVector NextPoint = DinosaurMemory.PatrolPoints[DinosaurMemory.CurrentPatrolIndex];
    
    // Advance to next patrol point
    DinosaurMemory.CurrentPatrolIndex = (DinosaurMemory.CurrentPatrolIndex + 1) % DinosaurMemory.PatrolPoints.Num();
    
    return NextPoint;
}

void UNPC_DinosaurBehaviorTree::UpdatePlayerMemory(FVector PlayerLocation)
{
    DinosaurMemory.LastKnownPlayerLocation = PlayerLocation;
    DinosaurMemory.LastPlayerSightTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: Updated player memory at %s"), 
           *PlayerLocation.ToString());
}

void UNPC_DinosaurBehaviorTree::AddPatrolPoint(FVector Point)
{
    DinosaurMemory.PatrolPoints.Add(Point);
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: Added patrol point %s"), 
           *Point.ToString());
}

void UNPC_DinosaurBehaviorTree::ClearMemory()
{
    DinosaurMemory.LastKnownPlayerLocation = FVector::ZeroVector;
    DinosaurMemory.LastPlayerSightTime = 0.0f;
    DinosaurMemory.LastFeedTime = 0.0f;
    DinosaurMemory.CurrentPatrolIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: Memory cleared"));
}

void UNPC_DinosaurBehaviorTree::OnStateChanged(ENPC_DinosaurState OldState, ENPC_DinosaurState NewState)
{
    // Handle state transition logic
    switch (NewState)
    {
        case ENPC_DinosaurState::Idle:
            // Reset speeds, clear targets
            break;
            
        case ENPC_DinosaurState::Patrol:
            // Set patrol speed, get next patrol point
            break;
            
        case ENPC_DinosaurState::Hunt:
            // Set chase speed, focus on player
            break;
            
        case ENPC_DinosaurState::Flee:
            // Set flee speed, run from threat
            break;
            
        case ENPC_DinosaurState::Feed:
            // Stop movement, play feeding animation
            DinosaurMemory.LastFeedTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
            break;
            
        case ENPC_DinosaurState::Sleep:
            // Stop movement, reduce awareness
            break;
            
        case ENPC_DinosaurState::Territorial:
            // Aggressive posture, defend territory
            break;
    }
}