// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../Characters/CharacterArtistSystem.h"
#include "../Animation/AnimationSystem.h"
#include "NPCBehaviorSystem.generated.h"

/**
 * Core Personality Traits that drive NPC behavior
 * These are permanent characteristics that define who the NPC is
 */
UENUM(BlueprintType)
enum class EPersonalityTrait : uint8
{
    // Primary Traits (Big Five + Survival-specific)
    Openness            UMETA(DisplayName = "Openness to Experience"),
    Conscientiousness   UMETA(DisplayName = "Conscientiousness"),
    Extraversion        UMETA(DisplayName = "Extraversion"),
    Agreeableness       UMETA(DisplayName = "Agreeableness"),
    Neuroticism         UMETA(DisplayName = "Neuroticism"),
    
    // Survival-Specific Traits
    Cautiousness        UMETA(DisplayName = "Cautiousness"),
    Resourcefulness     UMETA(DisplayName = "Resourcefulness"),
    Leadership          UMETA(DisplayName = "Leadership"),
    Adaptability        UMETA(DisplayName = "Adaptability"),
    RiskTolerance       UMETA(DisplayName = "Risk Tolerance"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Current Emotional States that change based on circumstances
 * These are temporary and reactive to the world
 */
UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    // Basic Emotions
    Calm                UMETA(DisplayName = "Calm"),
    Anxious             UMETA(DisplayName = "Anxious"),
    Fearful             UMETA(DisplayName = "Fearful"),
    Curious             UMETA(DisplayName = "Curious"),
    Confident           UMETA(DisplayName = "Confident"),
    Frustrated          UMETA(DisplayName = "Frustrated"),
    
    // Survival-Specific States
    Desperate           UMETA(DisplayName = "Desperate"),
    Hopeful             UMETA(DisplayName = "Hopeful"),
    Resigned            UMETA(DisplayName = "Resigned"),
    Determined          UMETA(DisplayName = "Determined"),
    Panicked            UMETA(DisplayName = "Panicked"),
    Focused             UMETA(DisplayName = "Focused"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Physical and Mental Needs that drive behavior
 */
UENUM(BlueprintType)
enum class ENeedType : uint8
{
    // Physical Needs
    Hunger              UMETA(DisplayName = "Hunger"),
    Thirst              UMETA(DisplayName = "Thirst"),
    Sleep               UMETA(DisplayName = "Sleep"),
    Warmth              UMETA(DisplayName = "Warmth"),
    Safety              UMETA(DisplayName = "Safety"),
    Health              UMETA(DisplayName = "Health"),
    
    // Psychological Needs
    Companionship       UMETA(DisplayName = "Companionship"),
    Purpose             UMETA(DisplayName = "Purpose"),
    Stimulation         UMETA(DisplayName = "Mental Stimulation"),
    Autonomy            UMETA(DisplayName = "Autonomy"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Daily Activity Types that NPCs engage in
 */
UENUM(BlueprintType)
enum class EActivityType : uint8
{
    // Survival Activities
    ForagingFood        UMETA(DisplayName = "Foraging for Food"),
    CollectingWater     UMETA(DisplayName = "Collecting Water"),
    BuildingShelter     UMETA(DisplayName = "Building Shelter"),
    MakingTools         UMETA(DisplayName = "Making Tools"),
    
    // Maintenance Activities
    RestingRecovering   UMETA(DisplayName = "Resting/Recovering"),
    CleaningSelf        UMETA(DisplayName = "Cleaning Self"),
    MaintainingGear     UMETA(DisplayName = "Maintaining Gear"),
    OrganizingSpace     UMETA(DisplayName = "Organizing Space"),
    
    // Exploration Activities
    ScoutingArea        UMETA(DisplayName = "Scouting Area"),
    MappingTerrain      UMETA(DisplayName = "Mapping Terrain"),
    StudyingEnvironment UMETA(DisplayName = "Studying Environment"),
    SearchingResources  UMETA(DisplayName = "Searching Resources"),
    
    // Social Activities
    Conversing          UMETA(DisplayName = "Conversing"),
    SharingKnowledge    UMETA(DisplayName = "Sharing Knowledge"),
    TeachingSkills      UMETA(DisplayName = "Teaching Skills"),
    SeekingHelp         UMETA(DisplayName = "Seeking Help"),
    
    // Contemplative Activities
    Thinking            UMETA(DisplayName = "Thinking/Planning"),
    Observing           UMETA(DisplayName = "Observing Environment"),
    Remembering         UMETA(DisplayName = "Remembering Past"),
    Worrying            UMETA(DisplayName = "Worrying"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Memory Types that NPCs can form and recall
 */
UENUM(BlueprintType)
enum class EMemoryType : uint8
{
    // Location Memories
    SafePlace           UMETA(DisplayName = "Safe Place"),
    DangerousArea       UMETA(DisplayName = "Dangerous Area"),
    ResourceLocation    UMETA(DisplayName = "Resource Location"),
    Landmark            UMETA(DisplayName = "Landmark"),
    
    // Event Memories
    ThreatEncounter     UMETA(DisplayName = "Threat Encounter"),
    SuccessfulForaging  UMETA(DisplayName = "Successful Foraging"),
    SocialInteraction   UMETA(DisplayName = "Social Interaction"),
    Discovery           UMETA(DisplayName = "Discovery"),
    
    // Relationship Memories
    PositiveInteraction UMETA(DisplayName = "Positive Interaction"),
    NegativeInteraction UMETA(DisplayName = "Negative Interaction"),
    HelpReceived        UMETA(DisplayName = "Help Received"),
    HelpGiven           UMETA(DisplayName = "Help Given"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Individual Need with current level and importance
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNeedState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    ENeedType NeedType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float CurrentLevel; // 0.0 = completely unsatisfied, 1.0 = completely satisfied

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float Importance; // How important this need is to this individual

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float DecayRate; // How fast this need decreases over time

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Need")
    float LastSatisfiedTime; // Game time when this need was last satisfied

    FNeedState()
    {
        NeedType = ENeedType::Hunger;
        CurrentLevel = 0.7f;
        Importance = 1.0f;
        DecayRate = 0.1f;
        LastSatisfiedTime = 0.0f;
    }
};

/**
 * Memory Entry that NPCs can store and recall
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    EMemoryType MemoryType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp; // When this memory was formed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float EmotionalWeight; // How emotionally significant this memory is

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Accuracy; // How accurate this memory is (degrades over time)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TMap<FString, float> AssociatedData; // Additional context data

    FMemoryEntry()
    {
        MemoryType = EMemoryType::SafePlace;
        Location = FVector::ZeroVector;
        Description = TEXT("");
        Timestamp = 0.0f;
        EmotionalWeight = 0.5f;
        Accuracy = 1.0f;
    }
};

/**
 * Daily Routine Entry - defines what NPCs do at different times
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float StartTime; // Time of day (0.0 = midnight, 0.5 = noon, 1.0 = midnight)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float Duration; // How long this activity lasts (in hours)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    EActivityType Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FVector PreferredLocation; // Where they like to do this activity

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float Priority; // How important this routine is (can be interrupted by needs)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<ENeedType> SatisfiedNeeds; // Which needs this activity addresses

    FRoutineEntry()
    {
        StartTime = 0.5f;
        Duration = 2.0f;
        Activity = EActivityType::ForagingFood;
        PreferredLocation = FVector::ZeroVector;
        Priority = 0.5f;
    }
};

/**
 * Complete Personality Profile for an NPC
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPersonalityProfile
{
    GENERATED_BODY()

    // Core Personality Traits (0.0 to 1.0 scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TMap<EPersonalityTrait, float> TraitLevels;

    // Background Information
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    FString Profession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    int32 Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FString> Skills; // What they're good at

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FString> Fears; // What they're afraid of

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Background")
    TArray<FString> Motivations; // What drives them

    // Behavioral Preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialPreference; // 0.0 = very introverted, 1.0 = very extroverted

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RiskTaking; // 0.0 = very risk-averse, 1.0 = very risk-taking

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Optimism; // 0.0 = very pessimistic, 1.0 = very optimistic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float Stubbornness; // 0.0 = very flexible, 1.0 = very stubborn

    FPersonalityProfile()
    {
        Name = TEXT("Unknown");
        Profession = TEXT("Unknown");
        Age = 35;
        SocialPreference = 0.5f;
        RiskTaking = 0.5f;
        Optimism = 0.5f;
        Stubbornness = 0.5f;
        
        // Initialize default trait levels
        TraitLevels.Add(EPersonalityTrait::Openness, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Conscientiousness, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Extraversion, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Agreeableness, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Neuroticism, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Cautiousness, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Resourcefulness, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Leadership, 0.5f);
        TraitLevels.Add(EPersonalityTrait::Adaptability, 0.5f);
        TraitLevels.Add(EPersonalityTrait::RiskTolerance, 0.5f);
    }
};

/**
 * NPC Behavior System - The core system that gives NPCs independent lives
 * 
 * This system implements:
 * - Personality-driven behavior using Big Five + survival traits
 * - Need-based decision making (hunger, thirst, safety, companionship)
 * - Memory system that influences future decisions
 * - Daily routines that NPCs follow when not responding to immediate needs
 * - Emotional states that change based on circumstances
 * - Social relationships and interactions
 * - Emergent behavior through trait combinations
 * 
 * Philosophy: Each NPC exists to live their own life, not to serve the player.
 * The player is an interruption in their ongoing existence.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PERSONALITY SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Personality")
    void InitializePersonality(const FPersonalityProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    FPersonalityProfile GenerateRandomPersonality(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Personality")
    float GetPersonalityTrait(EPersonalityTrait Trait) const;

    UFUNCTION(BlueprintCallable, Category = "Personality")
    void ModifyPersonalityTrait(EPersonalityTrait Trait, float Delta);

    // === EMOTIONAL SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void SetEmotionalState(EEmotionalState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    float GetEmotionalIntensity() const { return EmotionalIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Emotions")
    void ProcessEmotionalReaction(const FString& Stimulus, float Impact);

    // === NEEDS SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void InitializeNeeds();

    UFUNCTION(BlueprintCallable, Category = "Needs")
    float GetNeedLevel(ENeedType Need) const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SatisfyNeed(ENeedType Need, float Amount);

    UFUNCTION(BlueprintCallable, Category = "Needs")
    ENeedType GetMostUrgentNeed() const;

    UFUNCTION(BlueprintCallable, Category = "Needs")
    TArray<ENeedType> GetUrgentNeeds(float Threshold = 0.3f) const;

    // === MEMORY SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(EMemoryType Type, FVector Location, const FString& Description, float EmotionalWeight = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FMemoryEntry> RecallMemories(EMemoryType Type, float MaxAge = -1.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    FMemoryEntry GetBestMemoryOfType(EMemoryType Type) const;

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetOldMemories(float MaxAge);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasMemoryNear(FVector Location, float Radius, EMemoryType Type) const;

    // === ROUTINE SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Routine")
    void SetDailyRoutine(const TArray<FRoutineEntry>& Routine);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FRoutineEntry GetCurrentRoutineActivity() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    EActivityType GetCurrentActivity() const { return CurrentActivity; }

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void InterruptRoutine(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void ResumeRoutine();

    // === DECISION MAKING ===
    
    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    EActivityType DecideNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    FVector DecideDestination(EActivityType Activity);

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    float EvaluateActivityUrgency(EActivityType Activity) const;

    UFUNCTION(BlueprintCallable, Category = "Decision Making")
    bool ShouldInterruptCurrentActivity(EActivityType NewActivity) const;

    // === SOCIAL SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Social")
    void RegisterNearbyNPC(class ANPCCharacter* NPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UnregisterNearbyNPC(class ANPCCharacter* NPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipLevel(class ANPCCharacter* NPC) const;

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ModifyRelationship(class ANPCCharacter* NPC, float Delta, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Social")
    bool WantsToInteract() const;

    // === BEHAVIOR TREE INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void UpdateBlackboardValues();

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    class UBehaviorTree* GetBehaviorTreeForPersonality() const;

    UFUNCTION(BlueprintCallable, Category = "Behavior Tree")
    void SwitchBehaviorTree(class UBehaviorTree* NewTree);

    // === DEBUG AND MONITORING ===
    
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetCurrentBehaviorSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    TArray<FString> GetRecentDecisions() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBehaviorState() const;

protected:
    // === CORE DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FPersonalityProfile PersonalityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalStability; // How quickly emotions return to baseline

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs")
    TMap<ENeedType, FNeedState> CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FMemoryEntry> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories; // Limit to prevent infinite growth

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FRoutineEntry> DailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    EActivityType CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool RoutineInterrupted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineInterruptTime;

    // === SOCIAL DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<class ANPCCharacter*, float> Relationships; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<class ANPCCharacter*> NearbyNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastSocialInteraction;

    // === BEHAVIOR TREE DATA ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBehaviorTree* CurrentBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    TMap<FString, class UBehaviorTree*> PersonalityBehaviorTrees;

    // === DECISION HISTORY ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision History")
    TArray<FString> RecentDecisions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decision History")
    int32 MaxDecisionHistory;

    // === TIMING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float UpdateInterval; // How often to recalculate behavior

private:
    // Internal helper methods
    void UpdateNeeds(float DeltaTime);
    void UpdateEmotions(float DeltaTime);
    void UpdateMemories(float DeltaTime);
    void ProcessRoutine();
    void MakeDecisions();
    void LogDecision(const FString& Decision);
    float CalculatePersonalityInfluence(EActivityType Activity) const;
    float CalculateEmotionalInfluence(EActivityType Activity) const;
};