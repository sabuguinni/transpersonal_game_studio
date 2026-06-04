#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "Combat_DinosaurBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ECombat_DinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Feeding         UMETA(DisplayName = "Feeding"),
    Resting         UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BehaviorTreeKeys
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName TargetActorKey = "TargetActor";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName PatrolLocationKey = "PatrolLocation";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName BehaviorStateKey = "BehaviorState";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName AlertLevelKey = "AlertLevel";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName LastKnownLocationKey = "LastKnownLocation";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName PackLeaderKey = "PackLeader";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blackboard")
    FName FlankingPositionKey = "FlankingPosition";

    FCombat_BehaviorTreeKeys()
    {
        TargetActorKey = "TargetActor";
        PatrolLocationKey = "PatrolLocation";
        BehaviorStateKey = "BehaviorState";
        AlertLevelKey = "AlertLevel";
        LastKnownLocationKey = "LastKnownLocation";
        PackLeaderKey = "PackLeader";
        FlankingPositionKey = "FlankingPosition";
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurBehaviorTree();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* DinosaurBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    FCombat_BehaviorTreeKeys BlackboardKeys;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECombat_DinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    TArray<AActor*> PackMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    AActor* PackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void SetBehaviorState(ECombat_DinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void SetPatrolLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void SetAlertLevel(float AlertLevel);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPack(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void AddPackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void RemovePackMember(AActor* Member);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    ECombat_DinosaurBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

private:
    float LastStateChange;
    
    void UpdateBehaviorState();
    void UpdatePackBehavior();
    void UpdateBlackboardValues();
};