#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "QuestSystem.generated.h"

// ============================================================
// ENUMS — Quest state machine and objective types
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked          UMETA(DisplayName = "Locked"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    KillTarget          UMETA(DisplayName = "Kill Target"),
    CollectResource     UMETA(DisplayName = "Collect Resource"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    EscapeArea          UMETA(DisplayName = "Escape Area"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    SurviveDuration     UMETA(DisplayName = "Survive Duration"),
    TrackAnimal         UMETA(DisplayName = "Track Animal"),
    BuildStructure      UMETA(DisplayName = "Build Structure"),
    FollowMigration     UMETA(DisplayName = "Follow Migration")
};

UENUM(BlueprintType)
enum class EQuest_Category : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Defense         UMETA(DisplayName = "Defense"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Migration       UMETA(DisplayName = "Migration"),
    Survival        UMETA(DisplayName = "Survival")
};

UENUM(BlueprintType)
enum class EQuest_Difficulty : uint8
{
    Trivial     UMETA(DisplayName = "Trivial"),
    Easy        UMETA(DisplayName = "Easy"),
    Medium      UMETA(DisplayName = "Medium"),
    Hard        UMETA(DisplayName = "Hard"),
    Deadly      UMETA(DisplayName = "Deadly")
};

// ============================================================
// STRUCTS — Quest objective and quest data
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Objective")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    bool bIsOptional;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Objective")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Objective")
    FName TargetTag;

    FQuest_Objective()
        : ObjectiveID(NAME_None)
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsOptional(false)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(300.f)
        , TargetTag(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    TArray<FName> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FName UnlockRecipeID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FName UnlockAreaID;

    FQuest_Reward()
        : ExperiencePoints(0)
        , UnlockRecipeID(NAME_None)
        , UnlockAreaID(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Category Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Difficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FName> PrerequisiteQuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FName GiverNPCTag;

    FQuest_Data()
        : QuestID(NAME_None)
        , Category(EQuest_Category::Survival)
        , Difficulty(EQuest_Difficulty::Easy)
        , TimeLimit(0.f)
        , bHasTimeLimit(false)
        , GiverNPCTag(NAME_None)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_ActiveEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Active")
    FQuest_Data Data;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Active")
    EQuest_State State;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Active")
    float TimeRemaining;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Active")
    float TimeStarted;

    FQuest_ActiveEntry()
        : State(EQuest_State::Available)
        , TimeRemaining(0.f)
        , TimeStarted(0.f)
    {}
};

// ============================================================
// DELEGATE DECLARATIONS
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStarted, FName, QuestID, FQuest_Data, QuestData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestCompleted, FName, QuestID, FQuest_Reward, Reward);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestFailed, FName, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectiveUpdated, FName, QuestID, FQuest_Objective, Objective);

// ============================================================
// UCLASS — Quest Manager Actor
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Quest lifecycle ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(FName QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(FName QuestID);

    // ---- Objective tracking ----

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportObjectiveProgress(FName QuestID, FName ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportLocationReached(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportKill(FName TargetTag);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportItemCollected(FName ItemTag);

    UFUNCTION(BlueprintCallable, Category = "Quest|Objective")
    void ReportItemCrafted(FName RecipeID);

    // ---- Query ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_State GetQuestState(FName QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(FName QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_ActiveEntry> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_ActiveEntry> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Crowd integration ----

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnStampedeTriggered(FVector StampedeOrigin, float StampedeRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnPredatorDetected(FVector PredatorLocation, FName PredatorSpecies);

    // ---- Delegates ----

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestStarted OnQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnQuestFailed OnQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FOnObjectiveUpdated OnObjectiveUpdated;

private:
    // ---- Internal state ----

    UPROPERTY(VisibleAnywhere, Category = "Quest|State")
    TMap<FName, FQuest_ActiveEntry> ActiveQuestMap;

    UPROPERTY(VisibleAnywhere, Category = "Quest|State")
    TArray<FQuest_ActiveEntry> CompletedQuestList;

    UPROPERTY(VisibleAnywhere, Category = "Quest|State")
    TArray<FName> FailedQuestIDs;

    UPROPERTY(EditDefaultsOnly, Category = "Quest|Config")
    int32 MaxActiveQuests;

    void TickTimeLimitedQuests(float DeltaTime);
    void CheckObjectiveCompletion(FName QuestID);
    bool ArePrerequisitesMet(const FQuest_Data& QuestData) const;
    void RegisterDefaultQuests();
    FQuest_Data BuildRescueHuntersQuest() const;
    FQuest_Data BuildSurviveStampedeQuest() const;
    FQuest_Data BuildCraftStoneAxeQuest() const;
};
