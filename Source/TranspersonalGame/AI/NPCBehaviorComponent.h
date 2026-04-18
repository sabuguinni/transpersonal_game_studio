#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorTypes.h"
#include "NPCBehaviorComponent.generated.h"

/**
 * NPC BEHAVIOR COMPONENT - CORE NPC BEHAVIOR SYSTEM
 * 
 * This component handles all NPC behavior logic including:
 * - Personality-driven decision making
 * - Needs management and satisfaction
 * - Memory and relationship tracking
 * - Dynamic dialogue selection
 * - Daily routine execution
 * - Threat response and survival instincts
 * 
 * DESIGN PHILOSOPHY:
 * Every NPC is a living being with their own agenda, fears, and relationships.
 * The player is just another actor in their world, not the center of it.
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
    // CORE NPC PROPERTIES
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FString NPCName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FNPC_PersonalityProfile PersonalityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FNPC_Needs CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_ThreatLevel CurrentThreatLevel = ENPC_ThreatLevel::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    float StressLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    bool bIsConscious = true;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY AND RELATIONSHIPS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> LongTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxShortTermMemories = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxLongTermMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    // ═══════════════════════════════════════════════════════════════
    // SCHEDULE AND ROUTINE
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FNPC_DailySchedule DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    bool bFollowSchedule = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float ScheduleFlexibility = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector CurrentDestination = FVector::ZeroVector;

    // ═══════════════════════════════════════════════════════════════
    // DIALOGUE AND COMMUNICATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TArray<FNPC_DialogueLine> AvailableDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float LastDialogueTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueCooldown = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bCanSpeak = true;

    // ═══════════════════════════════════════════════════════════════
    // AI AND BEHAVIOR TREE INTEGRATION
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardData* BlackboardAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float AIUpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    float LastAIUpdateTime = 0.0f;

    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessMemories(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ExecuteCurrentBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void RespondToThreat(AActor* ThreatActor, ENPC_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemory(const FString& Description, const FVector& Location, float EmotionalWeight, AActor* RelatedActor = nullptr, ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::None);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateRelationship(AActor* TargetActor, float TrustChange, float RespectChange, float FearChange);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPC_Relationship* GetRelationship(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FString GetDialogue(ENPC_DialogueType DialogueType, AActor* TargetActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFlee() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFight() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldSeekHelp() const;

    // ═══════════════════════════════════════════════════════════════
    // SCHEDULE AND ROUTINE FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void UpdateSchedule(float CurrentTime);

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    FNPC_ScheduleEntry* GetCurrentScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void SetDestination(const FVector& NewDestination);

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void InterruptSchedule(ENPC_BehaviorState NewState, float Duration = 60.0f);

    // ═══════════════════════════════════════════════════════════════
    // PERSONALITY AND DECISION MAKING
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityInfluence(ENPC_PersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    bool MakePersonalityBasedDecision(float BaseChance, ENPC_PersonalityTrait InfluencingTrait, float TraitWeight = 1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    ENPC_BehaviorState ChooseBehaviorBasedOnNeeds();

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ModifyNeedsBasedOnPersonality(float DeltaTime);

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Utility")
    float GetCurrentGameHour() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    bool IsNearLocation(const FVector& Location, float Tolerance = 100.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> GetNearbyActors(float Radius = 1000.0f, TSubclassOf<AActor> ActorClass = nullptr) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    AActor* FindNearestActorOfType(TSubclassOf<AActor> ActorClass, float MaxDistance = 2000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void LogBehaviorDebug(const FString& Message) const;

private:
    // Internal state tracking
    float ScheduleInterruptEndTime = 0.0f;
    ENPC_BehaviorState PreInterruptState = ENPC_BehaviorState::Idle;
    float LastNeedsUpdateTime = 0.0f;
    float LastMemoryProcessTime = 0.0f;

    // Helper functions
    void InitializeDefaultSchedule();
    void InitializeDefaultDialogue();
    void CleanupOldMemories();
    void ConsolidateMemories();
    float CalculateStressLevel() const;
    void UpdateBlackboardValues();
};