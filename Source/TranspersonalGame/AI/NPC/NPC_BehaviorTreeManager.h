#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPC_BehaviorTreeManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Chase       UMETA(DisplayName = "Chase"),
    Attack      UMETA(DisplayName = "Attack"),
    Flee        UMETA(DisplayName = "Flee"),
    Feed        UMETA(DisplayName = "Feed"),
    Sleep       UMETA(DisplayName = "Sleep"),
    Investigate UMETA(DisplayName = "Investigate")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeHealthThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float IdleDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ChaseSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanFly = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bIsPredator = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core behavior management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StartBehaviorTree(UBehaviorTree* BehaviorTreeAsset);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void StopBehaviorTree();

    // Blackboard management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBlackboardValue(const FName& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBlackboardVector(const FName& KeyName, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBlackboardObject(const FName& KeyName, UObject* Value);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetBlackboardValue(const FName& KeyName) const;

    // Target management
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    AActor* GetTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ClearTarget();

    // Patrol system
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPatrolPoint(const FVector& Point);

    // Memory system
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RememberLocation(const FVector& Location, float Duration = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RememberActor(AActor* Actor, float Duration = 60.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasMemoryOf(AActor* Actor) const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FNPC_BehaviorConfig BehaviorConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    UBehaviorTree* DefaultBehaviorTree;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    int32 CurrentPatrolIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    float StateTimer;

    // Memory system
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    TMap<AActor*, float> ActorMemory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Behavior")
    TMap<FVector, float> LocationMemory;

    // AI Controller reference
    UPROPERTY()
    AAIController* AIController;

    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

private:
    void UpdateMemory(float DeltaTime);
    void UpdateBehaviorState(float DeltaTime);
    void InitializeAIComponents();
};