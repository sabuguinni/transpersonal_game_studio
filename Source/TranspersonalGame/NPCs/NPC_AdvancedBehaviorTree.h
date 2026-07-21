#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NPCBehaviorTypes.h"
#include "NPC_AdvancedBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorTreeNodeType : uint8
{
    Sequence        UMETA(DisplayName = "Sequence"),
    Selector        UMETA(DisplayName = "Selector"),
    Parallel        UMETA(DisplayName = "Parallel"),
    Decorator       UMETA(DisplayName = "Decorator"),
    Leaf            UMETA(DisplayName = "Leaf Action")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorTreeStatus : uint8
{
    Invalid         UMETA(DisplayName = "Invalid"),
    Success         UMETA(DisplayName = "Success"),
    Failure         UMETA(DisplayName = "Failure"),
    Running         UMETA(DisplayName = "Running"),
    Aborted         UMETA(DisplayName = "Aborted")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorTreeNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    ENPC_BehaviorTreeNodeType NodeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    FString NodeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    int32 Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    TArray<int32> ChildNodeIndices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    int32 ParentNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    ENPC_BehaviorTreeStatus LastExecutionStatus;

    FNPC_BehaviorTreeNode()
    {
        NodeType = ENPC_BehaviorTreeNodeType::Leaf;
        NodeName = TEXT("DefaultNode");
        Priority = 0;
        ExecutionTime = 0.0f;
        bIsActive = false;
        ParentNodeIndex = -1;
        LastExecutionStatus = ENPC_BehaviorTreeStatus::Invalid;
    }
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    AActor* OwnerActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    FVector LastKnownTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float DeltaTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    float TimeSinceLastUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    TMap<FString, float> FloatBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    TMap<FString, bool> BoolBlackboard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Context")
    TMap<FString, FVector> VectorBlackboard;

    FNPC_BehaviorContext()
    {
        OwnerActor = nullptr;
        TargetActor = nullptr;
        LastKnownTargetLocation = FVector::ZeroVector;
        DeltaTime = 0.0f;
        TimeSinceLastUpdate = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedBehaviorTree();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void ExecuteBehaviorTree(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void ResetBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void PauseBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void ResumeBehaviorTree();

    // Node Management
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    int32 AddBehaviorNode(const FNPC_BehaviorTreeNode& NewNode);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    bool RemoveBehaviorNode(int32 NodeIndex);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetNodeParent(int32 ChildIndex, int32 ParentIndex);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void AddChildToNode(int32 ParentIndex, int32 ChildIndex);

    // Execution Methods
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    ENPC_BehaviorTreeStatus ExecuteNode(int32 NodeIndex, FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    ENPC_BehaviorTreeStatus ExecuteSequenceNode(int32 NodeIndex, FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    ENPC_BehaviorTreeStatus ExecuteSelectorNode(int32 NodeIndex, FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    ENPC_BehaviorTreeStatus ExecuteParallelNode(int32 NodeIndex, FNPC_BehaviorContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    ENPC_BehaviorTreeStatus ExecuteLeafNode(int32 NodeIndex, FNPC_BehaviorContext& Context);

    // Blackboard Management
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetBlackboardFloat(const FString& Key, float Value);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    float GetBlackboardFloat(const FString& Key);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetBlackboardBool(const FString& Key, bool Value);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    bool GetBlackboardBool(const FString& Key);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetBlackboardVector(const FString& Key, const FVector& Value);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    FVector GetBlackboardVector(const FString& Key);

    // Utility Methods
    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    AActor* GetTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    bool IsNodeValid(int32 NodeIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    int32 GetRootNodeIndex() const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Behavior Tree")
    void SetRootNode(int32 NodeIndex);

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    TArray<FNPC_BehaviorTreeNode> BehaviorNodes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    FNPC_BehaviorContext BehaviorContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    int32 RootNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    int32 CurrentExecutingNodeIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    bool bIsBehaviorTreeActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    bool bIsPaused;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    float LastUpdateTime;

    // Execution State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    TArray<int32> ExecutionStack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    TMap<int32, ENPC_BehaviorTreeStatus> NodeExecutionStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Behavior Tree")
    TMap<int32, float> NodeExecutionTimes;

private:
    // Internal Methods
    void UpdateBehaviorContext(float DeltaTime);
    void CleanupFinishedNodes();
    bool ShouldExecuteNode(int32 NodeIndex) const;
    void OnNodeExecutionComplete(int32 NodeIndex, ENPC_BehaviorTreeStatus Status);
    int32 FindNextNodeToExecute();
    void ResetNodeExecution(int32 NodeIndex);

    // Predefined Behavior Actions
    ENPC_BehaviorTreeStatus ExecutePatrolAction(FNPC_BehaviorContext& Context);
    ENPC_BehaviorTreeStatus ExecuteChaseAction(FNPC_BehaviorContext& Context);
    ENPC_BehaviorTreeStatus ExecuteAttackAction(FNPC_BehaviorContext& Context);
    ENPC_BehaviorTreeStatus ExecuteFleeAction(FNPC_BehaviorContext& Context);
    ENPC_BehaviorTreeStatus ExecuteWaitAction(FNPC_BehaviorContext& Context);
    ENPC_BehaviorTreeStatus ExecuteSearchAction(FNPC_BehaviorContext& Context);

    // Condition Checks
    bool IsTargetInRange(float Range) const;
    bool IsTargetVisible() const;
    bool IsHealthLow() const;
    bool HasReachedDestination() const;
    bool IsPathBlocked() const;
};