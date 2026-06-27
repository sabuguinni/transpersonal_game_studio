// QuestManager.h
// Agent #14 — Quest & Mission Designer
// CYCLE: PROD_CYCLE_AUTO_20260627_012
// Quest system for Transpersonal Game Studio — Prehistoric Survival
// Manages 5 core quests: Survive Stampede, Follow Migration, Watering Hole Ambush,
// Hunt River Raptor, Gather Crafting Materials

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Survival    UMETA(DisplayName = "Survival"),    // Survive a dangerous event
    Hunt        UMETA(DisplayName = "Hunt"),        // Track and kill a target
    Gather      UMETA(DisplayName = "Gather"),      // Collect resources
    Explore     UMETA(DisplayName = "Explore"),     // Reach locations / follow path
    Defend      UMETA(DisplayName = "Defend")       // Protect camp / NPCs
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    SurviveTimer    UMETA(DisplayName = "Survive Timer"),
    FollowPath      UMETA(DisplayName = "Follow Path"),
    EscapeZone      UMETA(DisplayName = "Escape Zone")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimerDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(200.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , TimerDuration(0.0f)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StaminaBonus;

    FQuest_Reward()
        : ExperiencePoints(0)
        , HealthBonus(0.0f)
        , StaminaBonus(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TriggerActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ActiveObjectiveIndex;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Survival)
        , Status(EQuest_Status::Locked)
        , TriggerActorLabel(TEXT(""))
        , ActiveObjectiveIndex(0)
    {}
};

// ============================================================
// QUEST MANAGER ACTOR
// ============================================================

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Registry ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> CompletedQuestIDs;

    // ---- Quest Control ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ---- Trigger Handling ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnterTrigger(const FString& TriggerActorLabel);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerKilledDinosaur(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCollectedItem(const FString& ItemID, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedWaypoint(int32 WaypointIndex);

    // ---- Initialization ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void PrintQuestStatus();

private:
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    FQuest_Data* FindQuest(const FString& QuestID);

    // Survival timer for "Survive the Stampede"
    float StampedeTimerRemaining;
    bool bStampedeTimerActive;
};
