#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "NPCBehaviorTypes.h"
#include "NPC_BehaviorTreeManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0,
    Patrolling = 1,
    Hunting = 2,
    Feeding = 3,
    Fleeing = 4,
    Territorial = 5,
    Socializing = 6,
    Resting = 7,
    Migrating = 8
};

UENUM(BlueprintType)
enum class ENPC_BehaviorPriority : uint8
{
    VeryLow = 0,
    Low = 1,
    Normal = 2,
    High = 3,
    Critical = 4
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorTreeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    TSoftObjectPtr<UBehaviorTree> BehaviorTreeAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    ENPC_BehaviorPriority Priority = ENPC_BehaviorPriority::Normal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    float ActivationRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    float DeactivationRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    bool bRequiresLineOfSight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    TArray<FString> RequiredBlackboardKeys;

    FNPC_BehaviorTreeConfig()
    {
        BehaviorTreeAsset = nullptr;
        Priority = ENPC_BehaviorPriority::Normal;
        ActivationRange = 1000.0f;
        DeactivationRange = 1500.0f;
        bRequiresLineOfSight = false;
    }
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float StateTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float LastStateChangeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TMap<FString, float> BlackboardFloats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TMap<FString, bool> BlackboardBools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TMap<FString, FVector> BlackboardVectors;

    FNPC_BehaviorContext()
    {
        TargetActor = nullptr;
        TargetLocation = FVector::ZeroVector;
        CurrentState = ENPC_BehaviorState::Idle;
        StateTimer = 0.0f;
        LastStateChangeTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorTreeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core behavior tree management
    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    bool StartBehaviorTree(UBehaviorTree* BehaviorTree);

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void PauseBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void ResumeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    bool SwitchBehaviorTree(UBehaviorTree* NewBehaviorTree, ENPC_BehaviorPriority Priority = ENPC_BehaviorPriority::Normal);

    // Behavior state management
    UFUNCTION(BlueprintCallable, Category = "Behavior State")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintPure, Category = "Behavior State")
    ENPC_BehaviorState GetBehaviorState() const { return BehaviorContext.CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Behavior State")
    bool CanTransitionToState(ENPC_BehaviorState NewState) const;

    UFUNCTION(BlueprintPure, Category = "Behavior State")
    float GetTimeInCurrentState() const;

    // Blackboard management
    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardValueAsFloat(const FString& KeyName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardValueAsBool(const FString& KeyName, bool Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardValueAsVector(const FString& KeyName, FVector Value);

    UFUNCTION(BlueprintCallable, Category = "Blackboard")
    void SetBlackboardValueAsObject(const FString& KeyName, UObject* Value);

    UFUNCTION(BlueprintPure, Category = "Blackboard")
    float GetBlackboardValueAsFloat(const FString& KeyName) const;

    UFUNCTION(BlueprintPure, Category = "Blackboard")
    bool GetBlackboardValueAsBool(const FString& KeyName) const;

    UFUNCTION(BlueprintPure, Category = "Blackboard")
    FVector GetBlackboardValueAsVector(const FString& KeyName) const;

    // Behavior tree configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void RegisterBehaviorTree(const FString& BehaviorName, const FNPC_BehaviorTreeConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    bool ActivateBehaviorByName(const FString& BehaviorName);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBehaviorPriority(const FString& BehaviorName, ENPC_BehaviorPriority Priority);

    // Context and environment awareness
    UFUNCTION(BlueprintCallable, Category = "Context")
    void UpdateBehaviorContext(const FNPC_BehaviorContext& NewContext);

    UFUNCTION(BlueprintPure, Category = "Context")
    FNPC_BehaviorContext GetBehaviorContext() const { return BehaviorContext; }

    UFUNCTION(BlueprintCallable, Category = "Context")
    void SetTargetActor(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Context")
    void SetTargetLocation(FVector Location);

    // Utility functions
    UFUNCTION(BlueprintPure, Category = "Utility")
    bool IsValidBehaviorTree(UBehaviorTree* BehaviorTree) const;

    UFUNCTION(BlueprintPure, Category = "Utility")
    bool IsBehaviorTreeRunning() const;

    UFUNCTION(BlueprintPure, Category = "Utility")
    UBehaviorTree* GetCurrentBehaviorTree() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBehaviorState() const;

protected:
    // Core components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBlackboardComponent* BlackboardComponent;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    TMap<FString, FNPC_BehaviorTreeConfig> RegisteredBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    FString DefaultBehaviorName = TEXT("Idle");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    float StateTransitionCooldown = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Configuration")
    bool bAutoStartDefaultBehavior = true;

    // Current behavior state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FNPC_BehaviorContext BehaviorContext;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FString CurrentBehaviorName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ENPC_BehaviorPriority CurrentPriority;

    // Internal state
    UPROPERTY()
    UBehaviorTree* CurrentBehaviorTree;

    UPROPERTY()
    bool bIsPaused;

    UPROPERTY()
    float LastTransitionTime;

private:
    // Internal helper functions
    void InitializeBehaviorTreeComponent();
    void InitializeBlackboardComponent();
    void UpdateBehaviorContext(float DeltaTime);
    bool ValidateStateTransition(ENPC_BehaviorState FromState, ENPC_BehaviorState ToState) const;
    void OnBehaviorTreeFinished();
};