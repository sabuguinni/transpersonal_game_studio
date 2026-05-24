#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR COMPONENT
 * NPC Behavior Agent #11
 * 
 * Core component that drives NPC behavior, decision making, and social interactions.
 * Manages personality, needs, memory, relationships, and task execution.
 * Designed for realistic prehistoric tribal behavior patterns.
 */

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

    // ═══════════════════════════════════════════════════════════════
    // CORE NPC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_Profession Profession = ENPC_Profession::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float ExperienceLevel = 1.0f;

    // ═══════════════════════════════════════════════════════════════
    // PERSONALITY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<ENPC_PersonalityTrait> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Courage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Intelligence = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Sociability = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Aggression = 50.0f;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR STATE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_BehaviorState PreviousState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MinStateTime = 5.0f;

    // ═══════════════════════════════════════════════════════════════
    // NEEDS SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    FNPC_Needs CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float NeedsUpdateInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float HungerDecayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float ThirstDecayRate = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    float EnergyDecayRate = 0.5f;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayInterval = 30.0f;

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Relationships")
    int32 MaxRelationships = 20;

    // ═══════════════════════════════════════════════════════════════
    // TASK SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    TArray<FNPC_Task> TaskQueue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    FNPC_Task CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tasks")
    bool bHasActiveTask = false;

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    FNPC_PerceptionData PerceptionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PerceptionUpdateInterval = 0.5f;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> DialogueLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanSpeak = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LastSpeechTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float SpeechCooldown = 5.0f;

    // ═══════════════════════════════════════════════════════════════
    // PUBLIC BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_BehaviorState GetCurrentBehaviorState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanChangeBehaviorState() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddTask(const FNPC_Task& NewTask);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ClearTasks();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool HasPersonalityTrait(ENPC_PersonalityTrait Trait) const;

    // ═══════════════════════════════════════════════════════════════
    // NEEDS MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Needs")
    float GetMostUrgentNeed() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Needs")
    bool IsNeedCritical(float NeedValue) const { return NeedValue < 20.0f; }

    UFUNCTION(BlueprintCallable, Category = "NPC Needs")
    void SatisfyNeed(const FString& NeedType, float Amount);

    // ═══════════════════════════════════════════════════════════════
    // MEMORY MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FNPC_Memory& NewMemory);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_Memory> GetMemoriesOfType(ENPC_MemoryType MemoryType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMemoryOfLocation(FVector Location, float Radius = 500.0f) const;

    // ═══════════════════════════════════════════════════════════════
    // RELATIONSHIP MANAGEMENT
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    void AddOrUpdateRelationship(AActor* TargetActor, ENPC_RelationshipType RelationType, float AffectionChange = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    FNPC_Relationship* GetRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    TArray<FNPC_Relationship> GetRelationshipsOfType(ENPC_RelationshipType RelationType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    bool IsFriendly(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Relationships")
    bool IsHostile(AActor* TargetActor) const;

    // ═══════════════════════════════════════════════════════════════
    // PERCEPTION FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    void UpdatePerception();

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    bool CanSeeActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    bool CanHearActor(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    TArray<AActor*> GetVisibleActors() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Perception")
    AActor* GetNearestThreat() const;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    FString GetDialogueForSituation(ENPC_DialogueType DialogueType, AActor* TargetActor = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    bool CanSpeak() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Dialogue")
    void Speak(const FString& Text, ENPC_DialogueType DialogueType = ENPC_DialogueType::Information);

    // ═══════════════════════════════════════════════════════════════
    // DECISION MAKING
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    ENPC_BehaviorState DecideNextBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    FNPC_Task CreateTaskBasedOnNeeds();

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    float CalculateThreatLevel() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Decision")
    bool ShouldSeekShelter() const;

private:
    // ═══════════════════════════════════════════════════════════════
    // PRIVATE HELPER FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    void ProcessCurrentTask(float DeltaTime);
    void ExecuteNextTask();
    void HandleStateTransition();
    void UpdateAlertness(float DeltaTime);
    void CleanupOldMemories();
    void CleanupOldRelationships();
    
    // Timer handles
    FTimerHandle NeedsUpdateTimer;
    FTimerHandle MemoryDecayTimer;
    FTimerHandle PerceptionTimer;
};