// QuestSystemManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260628_001
// Manages quest state, objectives, and progression for the survival game

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "QuestSystemManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Active          UMETA(DisplayName = "Active"),
    ObjectivesMet   UMETA(DisplayName = "Objectives Met"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),
    Survival        UMETA(DisplayName = "Survival"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Gather          UMETA(DisplayName = "Gather"),
    Defense         UMETA(DisplayName = "Defense"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Migration       UMETA(DisplayName = "Migration")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItem     UMETA(DisplayName = "Collect Item"),
    SurviveDuration UMETA(DisplayName = "Survive Duration"),
    ProtectNPC      UMETA(DisplayName = "Protect NPC"),
    CraftItem       UMETA(DisplayName = "Craft Item"),
    AvoidPredator   UMETA(DisplayName = "Avoid Predator")
};

// ============================================================
// STRUCTS — must be at global scope
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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float LocationRadius;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
        , TargetLocation(FVector::ZeroVector)
        , LocationRadius(300.0f)
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
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ElapsedTime;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Inactive)
        , bIsMainQuest(false)
        , TimeLimit(0.0f)
        , ElapsedTime(0.0f)
    {}
};

// ============================================================
// QUEST SYSTEM MANAGER — World Subsystem
// ============================================================

UCLASS()
class TRANSPERSONALGAME_API UQuestSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Quest Lifecycle ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool AbandonQuest(const FString& QuestID);

    // ---- Objective Tracking ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnEnemyKilled(const FString& EnemyTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCollected(const FString& ItemTag);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnItemCrafted(const FString& ItemTag);

    // ---- Query ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FString> GetCompletedQuestIDs() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ---- Tick ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void TickQuestSystem(float DeltaTime);

    // ---- Stampede Integration ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnStampedeTriggered();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnStampedeStopped();

private:
    // Internal quest registry
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    UPROPERTY()
    TArray<FString> ActiveQuestIDs;

    UPROPERTY()
    TArray<FString> CompletedQuestIDs;

    UPROPERTY()
    TArray<FString> FailedQuestIDs;

    // Internal helpers
    void RegisterDefaultQuests();
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void CheckQuestCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    bool IsQuestActive(const FString& QuestID) const;
};
