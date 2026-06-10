#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "../../Core/SharedTypes.h"
#include "NPCBehaviorController.generated.h"

class UBehaviorTree;
class UBlackboardData;

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPC_MemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Importance;

    FNPC_MemoryEntry()
    {
        Location = FVector::ZeroVector;
        Timestamp = 0.0f;
        MemoryType = ENPC_MemoryType::Neutral;
        Importance = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastInteractionTime;

    FNPC_SocialRelation()
    {
        TargetActor = nullptr;
        TrustLevel = 0.0f;
        FearLevel = 0.0f;
        LastInteractionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    UNPC_BehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardData* DefaultBlackboard;

    // NPC Memory System
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> ShortTermMemory;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_MemoryEntry> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate;

    // Social Relationships
    UPROPERTY(BlueprintReadOnly, Category = "Social")
    TArray<FNPC_SocialRelation> SocialRelations;

    // Daily Routine System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineEndTime;

    UPROPERTY(BlueprintReadOnly, Category = "Routine")
    int32 CurrentPatrolIndex;

    // Perception Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngle;

public:
    // Memory Management Functions
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, ENPC_MemoryType Type, float Importance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateMemoryDecay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesOfType(ENPC_MemoryType Type);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FNPC_MemoryEntry GetNearestMemory(FVector Location, ENPC_MemoryType Type);

    // Social System Functions
    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelation(AActor* Target, float TrustDelta, float FearDelta);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_SocialRelation GetSocialRelation(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsFriendly(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool IsHostile(AActor* Target);

    // Routine Functions
    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetPatrolRoute(const TArray<FVector>& NewPatrolPoints);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FVector GetNextPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "Routine")
    bool IsWithinRoutineHours();

    // Perception Callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
    // Internal helper functions
    void InitializePerception();
    void ProcessMemoryConsolidation();
    void UpdateRoutineBehavior();
    void HandleEmergencyBehavior();

private:
    float LastMemoryUpdate;
    bool bInEmergencyMode;
    AActor* CurrentThreat;
};