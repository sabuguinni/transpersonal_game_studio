#include "NPC_DinosaurBehaviorTree.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    TRexDetectionRadius = 3000.0f;
    RaptorDetectionRadius = 1500.0f;
    PackCoordinationRadius = 2000.0f;
    BehaviorUpdateInterval = 0.5f;
    
    CreateDefaultBehaviorTrees();
}

void UNPC_DinosaurBehaviorTree::InitializeDinosaurAI(APawn* DinosaurPawn, ENPC_DinosaurSpecies Species)
{
    if (!DinosaurPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurBehaviorTree: Cannot initialize AI - DinosaurPawn is null"));
        return;
    }

    // Initialize dinosaur state
    FNPC_DinosaurState NewState;
    NewState.CurrentBehavior = ENPC_DinosaurBehavior::Idle;
    
    // Set species-specific defaults
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            NewState.AggressionLevel = 0.8f;
            NewState.HungerLevel = 0.6f;
            NewState.FearLevel = 0.1f;
            break;
        case ENPC_DinosaurSpecies::Raptor:
            NewState.AggressionLevel = 0.7f;
            NewState.HungerLevel = 0.5f;
            NewState.FearLevel = 0.3f;
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            NewState.AggressionLevel = 0.3f;
            NewState.HungerLevel = 0.4f;
            NewState.FearLevel = 0.6f;
            break;
        default:
            NewState.AggressionLevel = 0.5f;
            NewState.HungerLevel = 0.3f;
            NewState.FearLevel = 0.4f;
            break;
    }

    DinosaurStates.Add(DinosaurPawn, NewState);

    // Get AI Controller and set up behavior tree
    AAIController* AIController = Cast<AAIController>(DinosaurPawn->GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        SetupBlackboardKeys(AIController->GetBlackboardComponent(), NewState);
        
        // Start appropriate behavior tree based on species
        UBehaviorTree* SelectedBT = nullptr;
        switch (Species)
        {
            case ENPC_DinosaurSpecies::TRex:
                SelectedBT = TRexBehaviorTree;
                break;
            case ENPC_DinosaurSpecies::Raptor:
                SelectedBT = RaptorBehaviorTree;
                break;
            default:
                SelectedBT = HerbivoreBehaviorTree;
                break;
        }
        
        if (SelectedBT)
        {
            AIController->RunBehaviorTree(SelectedBT);
            UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: Started behavior tree for %s"), *DinosaurPawn->GetName());
        }
    }
}

void UNPC_DinosaurBehaviorTree::UpdateDinosaurBehavior(APawn* DinosaurPawn, const FNPC_DinosaurState& NewState)
{
    if (!DinosaurPawn || !DinosaurStates.Contains(DinosaurPawn))
    {
        return;
    }

    FNPC_DinosaurState& CurrentState = DinosaurStates[DinosaurPawn];
    CurrentState = NewState;

    // Update blackboard with new state
    AAIController* AIController = Cast<AAIController>(DinosaurPawn->GetController());
    if (AIController && AIController->GetBlackboardComponent())
    {
        SetupBlackboardKeys(AIController->GetBlackboardComponent(), CurrentState);
    }
}

FNPC_DinosaurState UNPC_DinosaurBehaviorTree::GetDinosaurState(APawn* DinosaurPawn)
{
    if (DinosaurStates.Contains(DinosaurPawn))
    {
        return DinosaurStates[DinosaurPawn];
    }
    
    return FNPC_DinosaurState();
}

void UNPC_DinosaurBehaviorTree::SetDinosaurTarget(APawn* DinosaurPawn, AActor* NewTarget)
{
    if (!DinosaurPawn || !DinosaurStates.Contains(DinosaurPawn))
    {
        return;
    }

    FNPC_DinosaurState& State = DinosaurStates[DinosaurPawn];
    State.CurrentTarget = NewTarget;
    
    if (NewTarget)
    {
        State.CurrentBehavior = ENPC_DinosaurBehavior::Hunting;
        State.LastKnownThreatLocation = NewTarget->GetActorLocation();
        UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: %s targeting %s"), *DinosaurPawn->GetName(), *NewTarget->GetName());
    }
    else
    {
        State.CurrentBehavior = ENPC_DinosaurBehavior::Idle;
    }

    UpdateDinosaurBehavior(DinosaurPawn, State);
}

void UNPC_DinosaurBehaviorTree::TriggerDinosaurAlert(APawn* DinosaurPawn, FVector ThreatLocation, float ThreatLevel)
{
    if (!DinosaurPawn || !DinosaurStates.Contains(DinosaurPawn))
    {
        return;
    }

    FNPC_DinosaurState& State = DinosaurStates[DinosaurPawn];
    State.LastKnownThreatLocation = ThreatLocation;
    State.FearLevel = FMath::Clamp(State.FearLevel + ThreatLevel, 0.0f, 1.0f);
    
    // Determine behavior based on threat level and dinosaur characteristics
    if (ThreatLevel > 0.7f && State.FearLevel > State.AggressionLevel)
    {
        State.CurrentBehavior = ENPC_DinosaurBehavior::Fleeing;
    }
    else if (ThreatLevel > 0.3f)
    {
        State.CurrentBehavior = ENPC_DinosaurBehavior::Alert;
    }

    UpdateDinosaurBehavior(DinosaurPawn, State);
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurBehaviorTree: %s alerted to threat at %s"), 
           *DinosaurPawn->GetName(), *ThreatLocation.ToString());
}

bool UNPC_DinosaurBehaviorTree::IsInPackFormation(APawn* DinosaurPawn)
{
    if (!DinosaurPawn)
    {
        return false;
    }

    TArray<APawn*> PackMembers = GetNearbyPackMembers(DinosaurPawn, PackCoordinationRadius);
    return PackMembers.Num() >= 2; // At least 2 other pack members nearby
}

TArray<APawn*> UNPC_DinosaurBehaviorTree::GetNearbyPackMembers(APawn* DinosaurPawn, float SearchRadius)
{
    TArray<APawn*> PackMembers;
    
    if (!DinosaurPawn)
    {
        return PackMembers;
    }

    FVector DinosaurLocation = DinosaurPawn->GetActorLocation();
    
    // Find all pawns within search radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(DinosaurPawn->GetWorld(), APawn::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        APawn* OtherPawn = Cast<APawn>(Actor);
        if (OtherPawn && OtherPawn != DinosaurPawn)
        {
            float Distance = FVector::Dist(DinosaurLocation, OtherPawn->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                // Check if it's the same species (simplified check by name)
                FString DinosaurName = DinosaurPawn->GetName().ToLower();
                FString OtherName = OtherPawn->GetName().ToLower();
                
                if ((DinosaurName.Contains(TEXT("raptor")) && OtherName.Contains(TEXT("raptor"))) ||
                    (DinosaurName.Contains(TEXT("rex")) && OtherName.Contains(TEXT("rex"))))
                {
                    PackMembers.Add(OtherPawn);
                }
            }
        }
    }
    
    return PackMembers;
}

void UNPC_DinosaurBehaviorTree::CreateDefaultBehaviorTrees()
{
    // In a real implementation, these would be loaded from assets
    // For now, we'll set them to nullptr and create them in Blueprint
    TRexBehaviorTree = nullptr;
    RaptorBehaviorTree = nullptr;
    HerbivoreBehaviorTree = nullptr;
    DinosaurBlackboard = nullptr;
}

void UNPC_DinosaurBehaviorTree::SetupBlackboardKeys(UBlackboardComponent* BlackboardComp, const FNPC_DinosaurState& State)
{
    if (!BlackboardComp)
    {
        return;
    }

    // Set blackboard values based on current state
    BlackboardComp->SetValueAsEnum(TEXT("CurrentBehavior"), static_cast<uint8>(State.CurrentBehavior));
    BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), State.AggressionLevel);
    BlackboardComp->SetValueAsFloat(TEXT("HungerLevel"), State.HungerLevel);
    BlackboardComp->SetValueAsFloat(TEXT("FearLevel"), State.FearLevel);
    BlackboardComp->SetValueAsVector(TEXT("LastThreatLocation"), State.LastKnownThreatLocation);
    
    if (State.CurrentTarget)
    {
        BlackboardComp->SetValueAsObject(TEXT("TargetActor"), State.CurrentTarget);
    }
    else
    {
        BlackboardComp->ClearValue(TEXT("TargetActor"));
    }
}

ENPC_DinosaurBehavior UNPC_DinosaurBehaviorTree::CalculateOptimalBehavior(const FNPC_DinosaurState& CurrentState, ENPC_DinosaurSpecies Species)
{
    // Simple behavior calculation based on state values
    if (CurrentState.FearLevel > 0.7f)
    {
        return ENPC_DinosaurBehavior::Fleeing;
    }
    
    if (CurrentState.HungerLevel > 0.8f && Species != ENPC_DinosaurSpecies::Triceratops)
    {
        return ENPC_DinosaurBehavior::Hunting;
    }
    
    if (CurrentState.AggressionLevel > 0.6f && CurrentState.CurrentTarget)
    {
        return ENPC_DinosaurBehavior::Hunting;
    }
    
    if (CurrentState.StateTimer > 30.0f) // Change behavior after 30 seconds
    {
        return ENPC_DinosaurBehavior::Patrolling;
    }
    
    return ENPC_DinosaurBehavior::Idle;
}