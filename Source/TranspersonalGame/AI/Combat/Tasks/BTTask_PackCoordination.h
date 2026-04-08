#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "../CombatAITypes.h"
#include "BTTask_PackCoordination.generated.h"

UENUM(BlueprintType)
enum class EPackTactic : uint8
{
    Surround,           // Spread out to surround target
    FlankLeft,          // Move to target's left
    FlankRight,         // Move to target's right
    Distract,           // Make noise and movement to distract
    DirectAssault,      // Charge directly at target
    Retreat,            // Fall back to regroup
    Ambush              // Hide and wait for opportunity
};

/**
 * Behavior Tree Task for pack coordination and tactical positioning
 * Allows pack members to coordinate their attacks and positioning
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBTTask_PackCoordination : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PackCoordination();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override;

protected:
    // Target for pack coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FBlackboardKeySelector TargetKey;

    // Pack leader reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    FBlackboardKeySelector PackLeaderKey;

    // Tactic to execute
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    EPackTactic PackTactic = EPackTactic::Surround;

    // Distance to maintain from target during coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float CoordinationDistance = 600.0f;

    // How long to maintain the tactic
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float TacticDuration = 5.0f;

    // Whether this AI should wait for pack leader's signal
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    bool bWaitForLeaderSignal = true;

    // Acceptable position tolerance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Coordination")
    float PositionTolerance = 150.0f;

private:
    struct FPackCoordinationMemory
    {
        EPackTactic CurrentTactic = EPackTactic::Surround;
        FVector TargetPosition = FVector::ZeroVector;
        FVector AssignedPosition = FVector::ZeroVector;
        float ElapsedTime = 0.0f;
        bool bPositionReached = false;
        bool bLeaderSignalReceived = false;
        int32 PackMemberIndex = 0;
    };

    FVector CalculateTacticalPosition(const FVector& TargetLocation, EPackTactic Tactic, int32 MemberIndex, int32 TotalMembers) const;
    bool IsPositionReached(const FVector& CurrentPosition, const FVector& TargetPosition, float Tolerance) const;
    void ExecuteTactic(UBehaviorTreeComponent& OwnerComp, FPackCoordinationMemory* Memory, float DeltaSeconds);
};