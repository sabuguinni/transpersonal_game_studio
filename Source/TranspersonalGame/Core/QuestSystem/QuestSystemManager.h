// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "QuestSystemManager.generated.h"

class UQuestDefinition;
class UMissionDefinition;
class ATranspersonalPlayerController;
class UQuestObjective;

/**
 * Quest State - tracks the current state of a quest
 */
UENUM(BlueprintType)
enum class EQuestState : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    Available           UMETA(DisplayName = "Available"),
    Active              UMETA(DisplayName = "Active"),
    Completed           UMETA(DisplayName = "Completed"),
    Failed              UMETA(DisplayName = "Failed"),
    Abandoned           UMETA(DisplayName = "Abandoned"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Quest Priority - determines quest importance and UI ordering
 */
UENUM(BlueprintType)
enum class EQuestPriority : uint8
{
    Critical            UMETA(DisplayName = "Critical"),        // Main story, survival critical
    High                UMETA(DisplayName = "High"),           // Important side quests
    Medium              UMETA(DisplayName = "Medium"),         // Standard quests
    Low                 UMETA(DisplayName = "Low"),            // Optional exploration
    Background          UMETA(DisplayName = "Background"),     // Passive objectives
    
    MAX                 UMETA(Hidden)
};

/**
 * Quest Category - thematic grouping of quests
 */
UENUM(BlueprintType)
enum class EQuestCategory : uint8
{
    // Core Survival Categories
    MainStory           UMETA(DisplayName = "Main Story"),      // Finding the gem, core narrative
    Survival            UMETA(DisplayName = "Survival"),        // Food, shelter, safety
    Exploration         UMETA(DisplayName = "Exploration"),     // Discovering the world
    
    // Interaction Categories
    DinosaurStudy       UMETA(DisplayName = "Dinosaur Study"), // Learning about creatures
    Domestication       UMETA(DisplayName = "Domestication"),   // Taming companions
    Territory           UMETA(DisplayName = "Territory"),       // Establishing safe zones
    
    // Crafting and Building
    Crafting            UMETA(DisplayName = "Crafting"),        // Tools, weapons, items
    Construction        UMETA(DisplayName = "Construction"),    // Base building, shelters
    Technology          UMETA(DisplayName = "Technology"),      // Advanced survival tech
    
    // Environmental
    Ecosystem           UMETA(DisplayName = "Ecosystem"),       // Understanding the world
    Weather             UMETA(DisplayName = "Weather"),         // Dealing with climate
    Seasonal            UMETA(DisplayName = "Seasonal"),        // Time-based challenges
    
    // Social and Narrative
    Memory              UMETA(DisplayName = "Memory"),          // Remembering the past
    Discovery           UMETA(DisplayName = "Discovery"),       // Uncovering mysteries
    Personal            UMETA(DisplayName = "Personal"),        // Character development
    
    MAX                 UMETA(Hidden)
};

/**
 * Objective Type - defines what the player needs to do
 */
UENUM(BlueprintType)
enum class EObjectiveType : uint8
{
    // Location-based objectives
    GoToLocation        UMETA(DisplayName = "Go to Location"),
    ExploreArea         UMETA(DisplayName = "Explore Area"),
    FindLocation        UMETA(DisplayName = "Find Location"),
    
    // Collection objectives
    CollectItems        UMETA(DisplayName = "Collect Items"),
    GatherResources     UMETA(DisplayName = "Gather Resources"),
    HuntCreature        UMETA(DisplayName = "Hunt Creature"),
    
    // Interaction objectives
    InteractWithObject  UMETA(DisplayName = "Interact with Object"),
    ObserveDinosaur     UMETA(DisplayName = "Observe Dinosaur"),
    TameDinosaur        UMETA(DisplayName = "Tame Dinosaur"),
    
    // Construction objectives
    BuildStructure      UMETA(DisplayName = "Build Structure"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    UpgradeBase         UMETA(DisplayName = "Upgrade Base"),
    
    // Survival objectives
    SurviveTime         UMETA(DisplayName = "Survive Time"),
    AvoidThreat         UMETA(DisplayName = "Avoid Threat"),
    DefendLocation      UMETA(DisplayName = "Defend Location"),
    
    // Knowledge objectives
    LearnAbout          UMETA(DisplayName = "Learn About"),
    Document            UMETA(DisplayName = "Document"),
    Research            UMETA(DisplayName = "Research"),
    
    // Narrative objectives
    WitnessEvent        UMETA(DisplayName = "Witness Event"),
    MakeChoice          UMETA(DisplayName = "Make Choice"),
    RememberPast        UMETA(DisplayName = "Remember Past"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Quest Objective - individual task within a quest
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestObjective
{
    GENERATED_BODY()

    // Basic objective info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objective\")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objective\")
    FText ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objective\")
    EObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objective\")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objective\")
    bool bIsHidden; // Hidden until revealed by story

    // Progress tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Progress\")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Progress\")
    int32 TargetProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Progress\")
    bool bIsCompleted;

    // Location data (for location-based objectives)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Location\")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Location\")
    float AcceptanceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Location\")
    FString LocationName;

    // Item/Resource data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Items\")
    FGameplayTagContainer RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Items\")
    TMap<FGameplayTag, int32> ItemQuantities;

    // Creature data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Creatures\")
    FGameplayTagContainer TargetCreatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Creatures\")
    TMap<FGameplayTag, int32> CreatureQuantities;

    // Time constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Time\")
    float TimeLimit; // 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Time\")
    float TimeRemaining;

    // Conditions and requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Conditions\")
    FGameplayTagContainer RequiredTags; // Player must have these tags

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Conditions\")
    FGameplayTagContainer ForbiddenTags; // Player must NOT have these tags

    // Rewards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    TMap<FGameplayTag, int32> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    FGameplayTagContainer TagRewards; // Knowledge, abilities, etc.

    FQuestObjective()
    {
        ObjectiveID = TEXT(\"\");
        ObjectiveDescription = FText::GetEmpty();
        ObjectiveType = EObjectiveType::GoToLocation;
        bIsOptional = false;
        bIsHidden = false;
        CurrentProgress = 0;
        TargetProgress = 1;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
        AcceptanceRadius = 100.0f;
        LocationName = TEXT(\"\");
        TimeLimit = 0.0f;
        TimeRemaining = 0.0f;
        ExperienceReward = 0;
    }
};

/**
 * Quest Definition - complete quest data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestDefinition
{
    GENERATED_BODY()

    // Basic quest info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    FText QuestSummary; // Short version for UI

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    EQuestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    EQuestPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Quest\")
    FGameplayTagContainer QuestTags;

    // Quest state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    EQuestState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    float QuestStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    float QuestCompletionTime;

    // Prerequisites and conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Prerequisites\")
    TArray<FString> PrerequisiteQuests; // Must complete these first

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Prerequisites\")
    FGameplayTagContainer RequiredPlayerTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Prerequisites\")
    int32 MinimumPlayerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Prerequisites\")
    float MinimumGameTime; // How long player must have been playing

    // Objectives
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objectives\")
    TArray<FQuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Objectives\")
    bool bRequireAllObjectives; // true = all required, false = any one

    // Narrative elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Narrative\")
    FText StartDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Narrative\")
    FText CompletionDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Narrative\")
    FText FailureDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Narrative\")
    FString StartCutscene; // Path to sequence asset

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Narrative\")
    FString CompletionCutscene;

    // Rewards
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    TMap<FGameplayTag, int32> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    FGameplayTagContainer TagRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Rewards\")
    TArray<FString> UnlockedQuests; // Quests that become available

    // Failure conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Failure\")
    float TimeLimit; // 0 = no limit

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Failure\")
    TArray<FString> FailureQuests; // Completing these fails this quest

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Failure\")
    FGameplayTagContainer FailureTags; // Getting these tags fails quest

    // Emotional design
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Emotion\")
    FText EmotionalGoal; // What should the player feel?

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Emotion\")
    FGameplayTagContainer MoodTags; // Tension, wonder, fear, etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Emotion\")
    float IntendedDuration; // How long should this take?

    FQuestDefinition()
    {
        QuestID = TEXT(\"\");
        QuestTitle = FText::GetEmpty();
        QuestDescription = FText::GetEmpty();
        QuestSummary = FText::GetEmpty();
        Category = EQuestCategory::Exploration;
        Priority = EQuestPriority::Medium;
        CurrentState = EQuestState::NotStarted;
        QuestStartTime = 0.0f;
        QuestCompletionTime = 0.0f;
        MinimumPlayerLevel = 0;
        MinimumGameTime = 0.0f;
        bRequireAllObjectives = true;
        StartDialogue = FText::GetEmpty();
        CompletionDialogue = FText::GetEmpty();
        FailureDialogue = FText::GetEmpty();
        StartCutscene = TEXT(\"\");
        CompletionCutscene = TEXT(\"\");
        ExperienceReward = 0;
        TimeLimit = 0.0f;
        EmotionalGoal = FText::GetEmpty();
        IntendedDuration = 300.0f; // 5 minutes default
    }
};

/**
 * Mission Instance - runtime instance of a quest
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMissionInstance
{
    GENERATED_BODY()

    // Reference to definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Mission\")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Mission\")
    FQuestDefinition QuestDefinition;

    // Runtime state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    EQuestState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    float LastUpdateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    int32 CompletedObjectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"State\")
    TArray<bool> ObjectiveCompletionStatus;

    // Dynamic data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Dynamic\")
    TMap<FString, FString> DynamicVariables; // For procedural content

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Dynamic\")
    FVector DynamicLocation; // For procedurally placed objectives

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Dynamic\")
    FGameplayTagContainer DynamicTags; // Runtime-added tags

    // Player context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Context\")
    FVector PlayerLocationAtStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Context\")
    float PlayerLevelAtStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Context\")
    FGameplayTagContainer PlayerTagsAtStart;

    FMissionInstance()
    {
        QuestID = TEXT(\"\");
        CurrentState = EQuestState::NotStarted;
        StartTime = 0.0f;
        LastUpdateTime = 0.0f;
        CompletedObjectives = 0;
        DynamicLocation = FVector::ZeroVector;
        PlayerLocationAtStart = FVector::ZeroVector;
        PlayerLevelAtStart = 0.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, const FString&, QuestID, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveCompleted, const FString&, QuestID, const FString&, ObjectiveID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, const FString&, QuestID);

/**
 * Quest System Manager - manages all quests and missions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuestSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Quest management
    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool StartQuest(const FString& QuestID, ATranspersonalPlayerController* PlayerController = nullptr);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool AbandonQuest(const FString& QuestID);

    // Objective management
    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Quest queries
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    TArray<FMissionInstance> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    TArray<FMissionInstance> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    TArray<FMissionInstance> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    FMissionInstance GetQuestInstance(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    EQuestState GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    bool IsQuestAvailable(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = \"Quest System\")
    bool IsQuestCompleted(const FString& QuestID) const;

    // Quest discovery and procedural generation
    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void DiscoverQuestsInArea(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    TArray<FString> GenerateContextualQuests(const FVector& PlayerLocation, const FGameplayTagContainer& PlayerTags);

    // Event handling
    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void HandleGameplayEvent(const FGameplayTag& EventTag, const FVector& Location, AActor* Instigator);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void HandlePlayerAction(const FGameplayTag& ActionTag, AActor* Target, const FGameplayTagContainer& Context);

    // Data management
    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void LoadQuestDefinitions(UDataTable* QuestDataTable);

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void SaveQuestProgress();

    UFUNCTION(BlueprintCallable, Category = \"Quest System\")
    void LoadQuestProgress();

    // Events
    UPROPERTY(BlueprintAssignable, Category = \"Quest Events\")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = \"Quest Events\")
    FOnObjectiveCompleted OnObjectiveCompleted;

    UPROPERTY(BlueprintAssignable, Category = \"Quest Events\")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = \"Quest Events\")
    FOnQuestFailed OnQuestFailed;

protected:
    // Quest storage
    UPROPERTY()
    TMap<FString, FQuestDefinition> QuestDefinitions;

    UPROPERTY()
    TMap<FString, FMissionInstance> ActiveMissions;

    UPROPERTY()
    TArray<FString> CompletedQuests;

    UPROPERTY()
    TArray<FString> FailedQuests;

    // System state
    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    ATranspersonalPlayerController* CachedPlayerController;

    // Internal methods
    void UpdateQuestObjectives(float DeltaTime);
    void CheckQuestPrerequisites();
    void ProcessQuestEvents();
    void UpdateQuestTimers(float DeltaTime);
    
    bool EvaluateQuestPrerequisites(const FQuestDefinition& QuestDef) const;
    void ProcessQuestCompletion(const FString& QuestID);
    void ProcessQuestFailure(const FString& QuestID);
    void GiveQuestRewards(const FQuestDefinition& QuestDef);
    void UnlockFollowupQuests(const FQuestDefinition& QuestDef);

    // Objective processing
    void UpdateLocationObjectives(FMissionInstance& Mission, float DeltaTime);
    void UpdateCollectionObjectives(FMissionInstance& Mission, float DeltaTime);
    void UpdateSurvivalObjectives(FMissionInstance& Mission, float DeltaTime);
    void UpdateObservationObjectives(FMissionInstance& Mission, float DeltaTime);

    // Procedural quest generation
    FQuestDefinition GenerateSurvivalQuest(const FVector& Location, const FGameplayTagContainer& Context);
    FQuestDefinition GenerateExplorationQuest(const FVector& Location, const FGameplayTagContainer& Context);
    FQuestDefinition GenerateDinosaurStudyQuest(const FVector& Location, const FGameplayTagContainer& Context);
    FQuestDefinition GenerateCraftingQuest(const FVector& Location, const FGameplayTagContainer& Context);
};"