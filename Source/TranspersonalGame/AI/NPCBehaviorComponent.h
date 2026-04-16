#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.generated.h"

class AActor;

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR COMPONENT
 * NPC Behavior Agent #11
 * 
 * Core component that drives NPC behavior, personality, needs, and social interactions.
 * This component gives life to NPCs through emergent behavior patterns.
 */

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ═══════════════════════════════════════════════════════════════
    // CORE NPC DATA
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Profession Profession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString TribeName;

    // ═══════════════════════════════════════════════════════════════
    // PERSONALITY & BEHAVIOR
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FNPC_Personality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState PreviousBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorStateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    bool bCanInterruptCurrentBehavior;

    // ═══════════════════════════════════════════════════════════════
    // NEEDS & DRIVES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    FNPC_Needs CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float NeedsDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float CriticalNeedsThreshold;

    // ═══════════════════════════════════════════════════════════════
    // SCHEDULE & ROUTINES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FNPC_DailySchedule DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float ScheduleFlexibility;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY & RELATIONSHIPS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_Relationship> Relationships;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE & COMMUNICATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LastDialogueTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldown;

    // ═══════════════════════════════════════════════════════════════
    // ENVIRONMENTAL AWARENESS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    TArray<AActor*> NearbyActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Awareness")
    TArray<AActor*> ThreatActors;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessSchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState DetermineNextBehavior();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ExecuteBehavior(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FString& EventDescription, const FVector& Location, float EmotionalIntensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetOldMemories();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_Memory> GetMemoriesAtLocation(const FVector& Location, float Radius);

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    void UpdateRelationship(const FString& TargetName, ENPC_RelationshipType Type, float AffectionChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    FNPC_Relationship GetRelationship(const FString& TargetName);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    bool HasRelationship(const FString& TargetName);

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FString GetDialogue(ENPC_DialogueType Type);

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    bool CanSpeak();

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void InitiateDialogue(AActor* Target);

    // ═══════════════════════════════════════════════════════════════
    // AWARENESS FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Awareness")
    void UpdateAwareness();

    UFUNCTION(BlueprintCallable, Category = "NPC Awareness")
    void DetectNearbyActors();

    UFUNCTION(BlueprintCallable, Category = "NPC Awareness")
    bool IsActorThreat(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Awareness")
    AActor* GetNearestThreat();

private:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL BEHAVIOR LOGIC
    // ═══════════════════════════════════════════════════════════════

    void HandleIdleBehavior(float DeltaTime);
    void HandleWorkingBehavior(float DeltaTime);
    void HandleSocializingBehavior(float DeltaTime);
    void HandleEatingBehavior(float DeltaTime);
    void HandleSleepingBehavior(float DeltaTime);
    void HandlePatrollingBehavior(float DeltaTime);
    void HandleFleeingBehavior(float DeltaTime);
    void HandleFightingBehavior(float DeltaTime);
    void HandleGatheringBehavior(float DeltaTime);
    void HandleHuntingBehavior(float DeltaTime);
    void HandleCraftingBehavior(float DeltaTime);
    void HandleTradingBehavior(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    float GetCurrentGameHour();
    bool IsNightTime();
    bool IsDayTime();
    FVector GetRandomLocationNearby(float Radius);
    void InitializeDefaultPersonality();
    void InitializeDefaultSchedule();
    void InitializeDefaultDialogue();

    // ═══════════════════════════════════════════════════════════════
    // TIMER HANDLES
    // ═══════════════════════════════════════════════════════════════

    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle NeedsUpdateTimer;
    FTimerHandle AwarenessUpdateTimer;
};