#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle = 0,
    Patrolling = 1,
    Investigating = 2,
    Fleeing = 3,
    Socializing = 4,
    Working = 5,
    Sleeping = 6,
    Alert = 7
};

UENUM(BlueprintType)
enum class ENPC_EmotionalState : uint8
{
    Calm = 0,
    Curious = 1,
    Afraid = 2,
    Angry = 3,
    Happy = 4,
    Sad = 5,
    Excited = 6,
    Tired = 7
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
    ENPC_EmotionalState EmotionalContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    FNPC_Memory()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        EmotionalContext = ENPC_EmotionalState::Calm;
        EventDescription = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<APawn> TargetPawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    FNPC_SocialRelationship()
    {
        TargetPawn = nullptr;
        TrustLevel = 0.5f;
        FearLevel = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Behavior Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_EmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float EnergyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float SocialNeed;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_Memory> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPC_Memory> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxLongTermMemories;

    // Social Relationships
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float SocialInteractionRadius;

    // Patrol System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    int32 CurrentPatrolIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    float PatrolSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Patrol")
    float PatrolWaitTime;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Core Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetEmotionalState(ENPC_EmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateAlertnessLevel(float DeltaAlertness);

    // Memory Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(FVector Location, ENPC_EmotionalState Emotion, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ConsolidateMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_Memory> GetMemoriesAtLocation(FVector Location, float Radius);

    // Social Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void UpdateSocialRelationship(APawn* TargetPawn, float TrustDelta, float FearDelta);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    FNPC_SocialRelationship GetSocialRelationship(APawn* TargetPawn);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    TArray<APawn*> GetNearbyNPCs();

    // Patrol Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Patrol")
    void SetPatrolPoints(const TArray<FVector>& NewPatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "NPC Patrol")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Patrol")
    void AdvancePatrolPoint();

    // AI Integration
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeBehaviorTree();

private:
    float LastMemoryConsolidationTime;
    float LastSocialUpdate;
    float PatrolTimer;

    void UpdateEnergyAndNeeds(float DeltaTime);
    void ProcessSocialInteractions(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void HandleEmotionalDecay(float DeltaTime);
};