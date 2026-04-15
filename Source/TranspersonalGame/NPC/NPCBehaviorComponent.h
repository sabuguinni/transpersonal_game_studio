#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "NPCBehaviorTypes.h"
#include "../Core/SharedTypes.h"
#include "NPCBehaviorComponent.generated.h"

class ANPCAIController;
class UBlackboardComponent;

/**
 * TRANSPERSONAL GAME STUDIO - NPC BEHAVIOR COMPONENT
 * NPC Behavior Agent #11
 * 
 * Core component that drives NPC autonomous behavior.
 * Handles daily routines, memory system, social interactions, and personality-driven decisions.
 * Each NPC exists to live their life - the player is just an interruption in that life.
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FString NPCName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_Profession Profession = ENPC_Profession::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_BehaviorState CurrentBehaviorState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    ENPC_Mood CurrentMood = ENPC_Mood::Content;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FNPC_Stats NPCStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Core")
    FNPC_Personality Personality;

    // ═══════════════════════════════════════════════════════════════
    // DAILY ROUTINE SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float CurrentGameHour = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    bool bFollowDailyRoutine = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float RoutineFlexibility = 0.3f;

    // ═══════════════════════════════════════════════════════════════
    // MEMORY SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_Memory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.01f;

    // ═══════════════════════════════════════════════════════════════
    // SOCIAL SYSTEM
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_Relationship> Relationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialRange = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastSocialInteraction = 0.0f;

    // ═══════════════════════════════════════════════════════════════
    // BEHAVIOR PARAMETERS
    // ═══════════════════════════════════════════════════════════════

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WanderRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DecisionUpdateInterval = 2.0f;

    // ═══════════════════════════════════════════════════════════════
    // CORE BEHAVIOR FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateBehaviorState();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ProcessDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPC_ActivityType GetCurrentScheduledActivity() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    // ═══════════════════════════════════════════════════════════════
    // MEMORY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(const FString& Description, const FVector& Location, float EmotionalWeight);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ProcessMemoryDecay();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FNPC_Memory> GetMemoriesOfType(const FString& EventType) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryOfLocation(const FVector& Location, float Tolerance = 100.0f) const;

    // ═══════════════════════════════════════════════════════════════
    // SOCIAL FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationships();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(const FString& TargetName, float AffinityChange, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    FNPC_Relationship GetRelationshipWith(const FString& TargetName) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> GetNearbyNPCs() const;

    // ═══════════════════════════════════════════════════════════════
    // DECISION MAKING
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    void EvaluateEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    float CalculatePersonalityInfluence(const FString& DecisionType) const;

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldInterruptRoutine() const;

    // ═══════════════════════════════════════════════════════════════
    // STATS AND MOOD
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void UpdateMood();

    UFUNCTION(BlueprintCallable, Category = "Stats")
    void ModifyStats(float HealthChange, float StaminaChange, float HungerChange, float ThirstChange);

    // ═══════════════════════════════════════════════════════════════
    // UTILITY FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void InitializeNPC(const FString& Name, ENPC_Profession NPCProfession);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetupDefaultSchedule();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetHomeLocation() const { return HomeLocation; }

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetHomeLocation(const FVector& NewHome) { HomeLocation = NewHome; }

private:
    // ═══════════════════════════════════════════════════════════════
    // INTERNAL DATA
    // ═══════════════════════════════════════════════════════════════

    FVector HomeLocation;
    FTimerHandle BehaviorUpdateTimer;
    FTimerHandle StatsUpdateTimer;
    float LastDecisionTime;
    
    // AI Controller reference
    ANPCAIController* NPCAIController;

    // ═══════════════════════════════════════════════════════════════
    // INTERNAL FUNCTIONS
    // ═══════════════════════════════════════════════════════════════

    void InitializeDefaults();
    void StartBehaviorTimers();
    void HandleBehaviorTransition(ENPC_BehaviorState NewState);
    float GetCurrentTimeOfDay() const;
    bool IsNightTime() const;
    void ProcessEnvironmentalFactors();
};