#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/SharedTypes.h"
#include "NPCBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrolling UMETA(DisplayName = "Patrolling"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sleeping UMETA(DisplayName = "Sleeping"),
    Eating UMETA(DisplayName = "Eating"),
    Crafting UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> LastSeenPlayerLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastPlayerInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> SafeLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float TrustLevel;

    FNPC_BehaviorMemory()
    {
        LastPlayerInteractionTime = 0.0f;
        FearLevel = 0.5f;
        TrustLevel = 0.3f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float WakeUpTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float SleepTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector HomeLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<ENPC_BehaviorState> ScheduledActivities;

    FNPC_DailyRoutine()
    {
        WakeUpTime = 6.0f;
        SleepTime = 22.0f;
        HomeLocation = FVector::ZeroVector;
    }
};

/**
 * Advanced NPC Behavior Tree Controller
 * Manages complex NPC behaviors including daily routines, memory, and social interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorTreeController : public AAIController
{
    GENERATED_BODY()

public:
    ANPCBehaviorTreeController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardComponent* BlackboardComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTreeComponent* BehaviorComp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_BehaviorMemory NPCMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_DailyRoutine DailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMemory(AActor* Actor, FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetDailyRoutine(const FNPC_DailyRoutine& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFlee(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessTimeOfDay(float CurrentTime);

protected:
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    void InitializeBehaviorTree();
    void UpdateBlackboardValues();
    void ProcessBehaviorLogic(float DeltaTime);
    
private:
    float LastUpdateTime;
    int32 CurrentPatrolIndex;
    bool bIsNightTime;
    
    void HandleIdleBehavior();
    void HandlePatrolBehavior();
    void HandleSocialBehavior();
    void HandleFleeingBehavior();
};