#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "AIController.h"
#include "../SharedTypes.h"
#include "NPCBehaviorTreeComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0,
    Patrolling = 1,
    Investigating = 2,
    Fleeing = 3,
    Socializing = 4,
    Working = 5,
    Resting = 6,
    Hunting = 7,
    Feeding = 8
};

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm = 0,
    Alert = 1,
    Fearful = 2,
    Aggressive = 3,
    Curious = 4,
    Tired = 5,
    Hungry = 6,
    Social = 7
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_BehaviorState AssociatedBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    FNPC_Memory()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        AssociatedBehavior = ENPC_BehaviorState::Idle;
        Importance = 1.0f;
        Description = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Affinity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Trust;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InteractionCount;

    FNPC_SocialRelation()
    {
        TargetActor = nullptr;
        Affinity = 0.0f;
        Trust = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_BehaviorTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorTreeComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* BehaviorTreeAsset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree")
    UBlackboardComponent* BlackboardComponent;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float StressLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float SocialNeed;

    // Memory System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_Memory> ShortTermMemory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_Memory> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Social System
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float MaxSocialRelations;

    // Patrol System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Patrol")
    float PatrolSpeed;

    // Motion Matching Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bUseMotionMatching;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MovementAnimationBlendTime;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void StopBehaviorTree();

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, ENPC_BehaviorState Behavior, float Importance, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_Memory GetMostImportantMemory();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_Memory> GetMemoriesNearLocation(FVector Location, float Radius);

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelations();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void AddSocialRelation(AActor* TargetActor, float InitialAffinity);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifySocialRelation(AActor* TargetActor, float AffinityChange, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelation GetSocialRelation(AActor* TargetActor);

    // Patrol Functions
    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void SetPatrolPoints(const TArray<FVector>& Points);

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    FVector GetCurrentPatrolTarget();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void AdvancePatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Patrol")
    void GenerateRandomPatrolPoints(int32 NumPoints, float Radius);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetDistanceToPlayer();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsPlayerInSight(float SightRange, float SightAngle);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void UpdateStressAndEnergy(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool ShouldReactToStimulus(float StimulusIntensity);

private:
    void InitializeBehaviorTree();
    void UpdateBlackboardValues();
    void ProcessMemories();
    void HandleStateTransitions();
    
    float LastMemoryUpdateTime;
    float LastSocialUpdateTime;
};