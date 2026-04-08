#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CircleTarget.generated.h"

/**
 * Behavior Tree Task that makes the AI circle around a target
 * Used for pack hunting tactics and maintaining distance while staying engaged
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UBTTask_CircleTarget : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_CircleTarget();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
    virtual uint16 GetInstanceMemorySize() const override;

protected:
    // Target to circle around
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    FBlackboardKeySelector TargetKey;

    // Distance to maintain from target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    float CircleRadius = 500.0f;

    // How fast to circle (degrees per second)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    float CircleSpeed = 45.0f;

    // Whether to circle clockwise or counter-clockwise
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    bool bClockwise = true;

    // Acceptable radius tolerance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    float RadiusTolerance = 100.0f;

    // Maximum time to spend circling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    float MaxCircleTime = 10.0f;

    // Whether to face the target while circling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Circle Target")
    bool bFaceTarget = true;

private:
    struct FCircleTargetMemory
    {
        float CurrentAngle = 0.0f;
        float ElapsedTime = 0.0f;
        FVector TargetLocation = FVector::ZeroVector;
        bool bInitialized = false;
    };

    FVector CalculateCirclePosition(const FVector& TargetLocation, float Angle, float Radius) const;
    void UpdateFacing(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation) const;
};