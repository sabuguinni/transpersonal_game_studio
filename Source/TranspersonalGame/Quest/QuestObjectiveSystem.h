#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "QuestObjectiveSystem.generated.h"

// ============================================================
// ENUMS — Quest Objective System
// ============================================================

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation       UMETA(DisplayName = "Reach Location"),
    CollectResource     UMETA(DisplayName = "Collect Resource"),
    HuntTarget          UMETA(DisplayName = "Hunt Target"),
    SurviveEvent        UMETA(DisplayName = "Survive Event"),
    EscapeZone          UMETA(DisplayName = "Escape Zone"),
    ProtectNPC          UMETA(DisplayName = "Protect NPC"),
    CraftItem           UMETA(DisplayName = "Craft Item"),
    TrackCreature       UMETA(DisplayName = "Track Creature"),
    ObserveCreature     UMETA(DisplayName = "Observe Creature"),
    BuildShelter        UMETA(DisplayName = "Build Shelter")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveState : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Optional    UMETA(DisplayName = "Optional")
};

UENUM(BlueprintType)
enum class EQuest_QuestState : uint8
{
    NotStarted  UMETA(DisplayName = "Not Started"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
    Abandoned   UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_RewardType : uint8
{
    CraftingRecipe  UMETA(DisplayName = "Crafting Recipe"),
    ResourceCache   UMETA(DisplayName = "Resource Cache"),
    SafeLocation    UMETA(DisplayName = "Safe Location"),
    NPCAlly         UMETA(DisplayName = "NPC Ally"),
    KnowledgeEntry  UMETA(DisplayName = "Knowledge Entry")
};

// ============================================================
// STRUCTS — Quest Objective System
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_ObjectiveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString DisplayText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveState State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsOptional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bShowOnMap;

    FQuest_ObjectiveData()
        : ObjectiveID(TEXT(""))
        , DisplayText(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , State(EQuest_ObjectiveState::Inactive)
        , RequiredCount(1)
        , CurrentCount(0)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(300.0f)
        , bIsOptional(false)
        , bShowOnMap(true)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_RewardData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_RewardType RewardType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RewardQuantity;

    FQuest_RewardData()
        : RewardType(EQuest_RewardType::ResourceCache)
        , RewardID(TEXT(""))
        , RewardDescription(TEXT(""))
        , RewardQuantity(1)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestState QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_ObjectiveData> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_RewardData> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_QuestData()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestState(EQuest_QuestState::NotStarted)
        , GiverNPCID(TEXT(""))
        , bIsMainQuest(false)
        , TimeLimit(0.0f)
    {}
};

// ============================================================
// UCLASS — Quest Objective Manager Actor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ObjectiveManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ObjectiveManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Registration ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_QuestData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    // ---- Objective Progress ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailObjective(const FString& QuestID, const FString& ObjectiveID);

    // ---- Location Tracking ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerInObjectiveRadius(const FString& QuestID, const FString& ObjectiveID, const FVector& PlayerLocation) const;

    // ---- Query ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool GetQuestData(const FString& QuestID, FQuest_QuestData& OutData) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    // ---- Reward Delivery ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void DeliverRewards(const FString& QuestID);

    // ---- Predefined Quest Builders ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void BuildQuest_FlintRun();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void BuildQuest_StampedeEscape();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void BuildQuest_WaterSource();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void BuildQuest_RaptorAmbush();

    // ---- Properties ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    bool bAutoTrackNearestObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float LocationCheckInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    int32 TotalQuestsCompleted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    int32 TotalQuestsFailed;

private:
    UPROPERTY()
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY()
    TArray<FQuest_QuestData> CompletedQuests;

    float LocationCheckTimer;

    bool CheckAllObjectivesComplete(const FQuest_QuestData& Quest) const;
    void FinalizeQuestCompletion(FQuest_QuestData& Quest);
};
