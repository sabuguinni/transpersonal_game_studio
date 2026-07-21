#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/ActorComponent.h"
#include "NPC_AdvancedBehaviorTree.generated.h"

// NPC behavior states for primitive humans
UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Patrol UMETA(DisplayName = "Patrol"),
    Hunt UMETA(DisplayName = "Hunt"),
    Gather UMETA(DisplayName = "Gather"),
    Flee UMETA(DisplayName = "Flee"),
    Social UMETA(DisplayName = "Social"),
    Sleep UMETA(DisplayName = "Sleep"),
    Alert UMETA(DisplayName = "Alert"),
    Combat UMETA(DisplayName = "Combat"),
    Follow UMETA(DisplayName = "Follow")
};

// NPC priority system for decision making
UENUM(BlueprintType)
enum class ENPC_Priority : uint8
{
    VeryLow UMETA(DisplayName = "Very Low"),
    Low UMETA(DisplayName = "Low"),
    Normal UMETA(DisplayName = "Normal"),
    High UMETA(DisplayName = "High"),
    Critical UMETA(DisplayName = "Critical")
};

// NPC memory entry for behavior decisions
USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    ENPC_Priority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        TargetActor = nullptr;
        Priority = ENPC_Priority::Normal;
        Timestamp = 0.0f;
        Description = TEXT("");
    }
};

// Advanced behavior tree controller for NPCs
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_AdvancedBehaviorTreeController : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_AdvancedBehaviorTreeController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Behavior tree asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // Blackboard asset
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset;

    // AI Perception component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Current behavior state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    // Memory system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    // Social behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> TribeMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TribalLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    FVector HomeLocation;

public:
    // Behavior tree functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    // Memory functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryEntry(const FNPC_MemoryEntry& NewEntry);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearOldMemories(float MaxAge);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesByPriority(ENPC_Priority MinPriority);

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Decision making
    UFUNCTION(BlueprintCallable, Category = "AI")
    ENPC_BehaviorState DecideNextBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* FindNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "AI")
    AActor* FindNearestResource();

    // Social functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void RegisterTribeMember(AActor* NewMember);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void SetTribalLeader(AActor* Leader);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsNearTribe();

private:
    void InitializePerception();
    void UpdateBehaviorState(float DeltaTime);
    void ProcessMemories();
    
    float LastMemoryCleanup;
    float StateChangeTimer;
};

// Behavior tree component for advanced NPC behavior
UCLASS(BlueprintType, ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_AdvancedBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_AdvancedBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior tree controller reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    ANPC_AdvancedBehaviorTreeController* BehaviorController;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bAutoStartBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    ENPC_BehaviorState InitialBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float BehaviorUpdateInterval;

public:
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void InitializeBehavior();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorController(ANPC_AdvancedBehaviorTreeController* Controller);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    ANPC_AdvancedBehaviorTreeController* GetBehaviorController() const;

private:
    float LastBehaviorUpdate;
};