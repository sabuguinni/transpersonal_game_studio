#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Core/SharedTypes.h"
#include "UNPC_BehaviorController.generated.h"

class UNPC_MemoryComponent;
class ANPC_Character;

/**
 * NPC Behavior Controller - Advanced AI Controller for NPCs
 * Handles behavior trees, perception, memory, and social interactions
 * Each NPC has unique personality traits and behavioral patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorController : public AAIController
{
    GENERATED_BODY()

public:
    UNPC_BehaviorController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void Possess(APawn* InPawn) override;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR TREE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Behavior")
    class UBlackboardComponent* BlackboardComponent;

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Perception")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float LoseSightRadius = 1600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngleDegrees = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRange = 1200.0f;

    // ═══════════════════════════════════════════════════════════════
    // NPC PERSONALITY & TRAITS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Curiosity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Sociability = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Fearfulness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Personality")
    float Loyalty = 0.5f;

    // ═══════════════════════════════════════════════════════════════
    // DAILY ROUTINE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FVector> PatrolPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineStartTime = 6.0f; // 6 AM

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineEndTime = 22.0f; // 10 PM

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bFollowsDailyRoutine = true;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY COMPONENT
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Memory")
    UNPC_MemoryComponent* MemoryComponent;

    // ═══════════════════════════════════════════════════════════════
    // SOCIAL INTERACTION SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<ANPC_Character*> KnownNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<ANPC_Character*, float> RelationshipValues; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRadius = 300.0f;

public:
    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR CONTROL FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void SetBehaviorTree(UBehaviorTree* NewBehaviorTree);

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION CALLBACKS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // ═══════════════════════════════════════════════════════════════
    // SOCIAL INTERACTION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InteractWithNPC(ANPC_Character* OtherNPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipValue(ANPC_Character* OtherNPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(ANPC_Character* OtherNPC, float Delta);

    // ═══════════════════════════════════════════════════════════════
    // DAILY ROUTINE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void StartDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void UpdateRoutineBasedOnTime();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FVector GetCurrentRoutineTarget();

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    bool IsPlayerNearby(float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    AActor* GetNearestThreat();

    UFUNCTION(BlueprintCallable, Category = "AI Utility")
    void SetPersonalityTrait(const FString& TraitName, float Value);

private:
    // Internal state tracking
    float LastRoutineUpdateTime;
    int32 CurrentPatrolIndex;
    bool bIsInCombat;
    bool bIsInteracting;
    
    // Setup functions
    void SetupPerception();
    void SetupBlackboard();
    void InitializePersonality();
};