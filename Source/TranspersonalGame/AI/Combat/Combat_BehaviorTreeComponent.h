#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/Engine.h"
#include "Combat_BehaviorTreeComponent.generated.h"

UENUM(BlueprintType)
enum class ECombat_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Defending   UMETA(DisplayName = "Defending"),
    Stalking    UMETA(DisplayName = "Stalking")
};

USTRUCT(BlueprintType)
struct FCombat_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    float FleeThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    bool bCanFormPacks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    int32 MaxPackSize = 5;

    FCombat_BehaviorConfig()
    {
        AggressionLevel = 0.5f;
        DetectionRange = 1500.0f;
        AttackRange = 300.0f;
        FleeThreshold = 0.3f;
        bCanFormPacks = false;
        MaxPackSize = 5;
    }
};

UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_BehaviorTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_BehaviorTreeComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void InitializeBehaviorTree(class UBehaviorTree* BehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SetBehaviorState(ECombat_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    ECombat_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    // Target Management
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    // Combat Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool ShouldAttackTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    bool ShouldFleeFromTarget(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    FVector GetTacticalPosition(AActor* Target) const;

    // Pack Behavior
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void JoinPack(UCombat_BehaviorTreeComponent* PackLeader);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void LeavePack();

    UFUNCTION(BlueprintPure, Category = "Combat Behavior")
    bool IsInPack() const { return PackMembers.Num() > 1 || PackLeader != nullptr; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Behavior")
    ECombat_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Behavior")
    FCombat_BehaviorConfig BehaviorConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Behavior")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Behavior")
    float LastStateChangeTime;

    // Pack System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    UCombat_BehaviorTreeComponent* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UCombat_BehaviorTreeComponent*> PackMembers;

    // Behavior Tree References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComp;

private:
    void UpdateBehaviorState(float DeltaTime);
    void UpdateTarget();
    void UpdatePackBehavior();
    
    float GetDistanceToTarget() const;
    bool IsTargetInAttackRange() const;
    bool IsTargetThreatening() const;
};