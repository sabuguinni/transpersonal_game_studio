// QuestSystemManager.h
// Quest & Mission Designer — Agent #14
// Transpersonal Game Studio — PROD_CYCLE_AUTO_20260622_009
// Prehistoric survival quest system: hunt, scout, defend, gather, rescue missions

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSystemManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive     UMETA(DisplayName = "Inactive"),
    Available    UMETA(DisplayName = "Available"),
    Active       UMETA(DisplayName = "Active"),
    Completed    UMETA(DisplayName = "Completed"),
    Failed       UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt         UMETA(DisplayName = "Hunt"),
    Scout        UMETA(DisplayName = "Scout"),
    Defend       UMETA(DisplayName = "Defend"),
    Gather       UMETA(DisplayName = "Gather"),
    Rescue       UMETA(DisplayName = "Rescue"),
    Survive      UMETA(DisplayName = "Survive"),
    Craft        UMETA(DisplayName = "Craft")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget       UMETA(DisplayName = "Kill Target"),
    ReachLocation    UMETA(DisplayName = "Reach Location"),
    CollectItem      UMETA(DisplayName = "Collect Item"),
    ProtectTarget    UMETA(DisplayName = "Protect Target"),
    SurviveTimer     UMETA(DisplayName = "Survive Timer"),
    CraftItem        UMETA(DisplayName = "Craft Item"),
    TrackAnimal      UMETA(DisplayName = "Track Animal")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
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
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
        , TargetLocation(FVector::ZeroVector)
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
    float BonusStamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float BonusHealth;

    FQuest_Reward()
        : ExperiencePoints(0)
        , BonusStamina(0.0f)
        , BonusHealth(0.0f)
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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverNPCID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Hunt)
        , QuestState(EQuest_State::Inactive)
        , QuestGiverNPCID(TEXT(""))
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
    {}
};

// ============================================================
// QUEST SYSTEM MANAGER ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Active quest registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> CompletedQuestIDs;

    // Time tracking for timed quests
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|State")
    TMap<FString, float> QuestTimers;

    // Quest management functions
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // Event notifications (called by other systems)
    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnDinosaurKilled(const FString& DinosaurSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnLocationReached(const FVector& Location, float AcceptanceRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnItemCollected(const FString& ItemID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnNPCProtected(const FString& NPCID);

    UFUNCTION(BlueprintCallable, Category = "Quest|Events")
    void OnItemCrafted(const FString& ItemID);

private:
    void RegisterQuest(const FQuest_Data& Quest);
    void CheckAllObjectivesComplete(const FString& QuestID);
    void TickQuestTimers(float DeltaTime);
    FQuest_Data* FindQuestByID(const FString& QuestID);
};
