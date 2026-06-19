#include "BTTask_QueryNPCMemory.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_QueryNPCMemory::UBTTask_QueryNPCMemory()
{
    NodeName = TEXT("Query NPC Memory");
    // This task does not tick — it reads once and returns
    bNotifyTick = false;

    // Accept Vector keys only
    TargetLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_QueryNPCMemory, TargetLocationKey));
}

EBTNodeResult::Type UBTTask_QueryNPCMemory::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    // Get the AI controller and its pawn
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (!ControlledPawn)
    {
        return EBTNodeResult::Failed;
    }

    // Find the NPCMemoryComponent on the pawn
    UNPCMemoryComponent* MemComp = ControlledPawn->FindComponentByClass<UNPCMemoryComponent>();
    if (!MemComp)
    {
        // No memory component — task fails gracefully (BT can fall through to default patrol)
        return EBTNodeResult::Failed;
    }

    // Query the appropriate location
    FVector QueryLocation = FVector::ZeroVector;
    bool bLocationFound = false;

    if (bWriteSafeLocation)
    {
        // Prey/NPC flee path: find the safest known location
        bLocationFound = MemComp->GetSafestKnownLocation(QueryLocation);
    }
    else
    {
        // Predator pursuit path: find the most threatening known location
        // Apply tag filter if set
        if (SourceTagFilter.IsValid())
        {
            // Filter memories by source tag before querying
            // GetMostThreateningLocation with tag filter
            bLocationFound = MemComp->GetMostThreateningLocation(QueryLocation, SourceTagFilter);
        }
        else
        {
            bLocationFound = MemComp->GetMostThreateningLocation(QueryLocation);
        }
    }

    if (!bLocationFound)
    {
        // No valid memory above threshold — task fails, BT moves to next branch
        return EBTNodeResult::Failed;
    }

    // Check threat threshold
    if (!bWriteSafeLocation && MemComp->CurrentThreatLevel < MinThreatThreshold)
    {
        return EBTNodeResult::Failed;
    }

    // Write the location to the Blackboard
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return EBTNodeResult::Failed;
    }

    Blackboard->SetValueAsVector(TargetLocationKey.SelectedKeyName, QueryLocation);

    return EBTNodeResult::Succeeded;
}

FString UBTTask_QueryNPCMemory::GetStaticDescription() const
{
    FString Mode = bWriteSafeLocation ? TEXT("SafeLocation") : TEXT("ThreatLocation");
    return FString::Printf(
        TEXT("Query NPCMemory [%s] → BB:%s (MinThreat:%.2f)"),
        *Mode,
        *TargetLocationKey.SelectedKeyName.ToString(),
        MinThreatThreshold
    );
}
