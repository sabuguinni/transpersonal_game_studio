#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NPCMemoryComponent.h"
#include "BTTask_QueryNPCMemory.generated.h"

/**
 * BTTask_QueryNPCMemory
 * Reads the owning pawn's UNPCMemoryComponent and writes the most threatening
 * known location to a Blackboard Vector key. Used by raptor/T-Rex BTs to
 * resume pursuit after losing line-of-sight.
 *
 * Usage in Behavior Tree:
 *   - Place after a "Lost Sight" decorator fires
 *   - Set TargetLocationKey to the BB key used by MoveTo tasks
 *   - Optionally set bWriteSafeLocation=true for prey-flee BTs
 */
UCLASS()
class TRANSPERSONALGAME_API UBTTask_QueryNPCMemory : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_QueryNPCMemory();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual FString GetStaticDescription() const override;

    // Blackboard key to write the queried location into
    UPROPERTY(EditAnywhere, Category = "Combat AI")
    FBlackboardKeySelector TargetLocationKey;

    // If true, writes the safest known location instead of the most threatening
    UPROPERTY(EditAnywhere, Category = "Combat AI")
    bool bWriteSafeLocation = false;

    // Minimum threat weight required to act on a memory (0.0 = any memory)
    UPROPERTY(EditAnywhere, Category = "Combat AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinThreatThreshold = 0.1f;

    // Tag filter — only consider memories from this source tag (empty = all)
    UPROPERTY(EditAnywhere, Category = "Combat AI")
    FGameplayTag SourceTagFilter;
};
