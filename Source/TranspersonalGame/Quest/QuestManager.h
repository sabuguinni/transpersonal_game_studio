#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Quest system for prehistoric survival game
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
    MainStory       UMETA(DisplayName = "Main Story"),
    Survival        UMETA(DisplayName = "Survival"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Crafting        UMETA(DisplayName = "Crafting"),
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
    CraftItem       UMETA(DisplayName = "Craft Item"),
    ObserveTarget   UMETA(DisplayName = "Observe Target"),
    EscapeZone      UMETA(DisplayName = "Escape Zone"),
    ProtectTarget   UMETA(DisplayName = "Protect Target"),
    FollowTarget    UMETA(DisplayName = "Follow Target")
};

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
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , TriggerRadius(300.f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
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
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString RewardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Data()
        : QuestType(EQuest_Type::Survival)
        , Status(EQuest_Status::Locked)
        , bIsMainQuest(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    // All registered quests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Data> AllQuests;

    // Currently active quest IDs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    // Completed quest IDs
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    // === QUEST MANAGEMENT ===

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressDelta);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // === TRIGGER DETECTION ===

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckLocationTriggers(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCollectedItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerCraftedItem(const FString& ItemName);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinosaurKilled(const FString& DinoSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerEnteredDangerZone(const FString& ZoneName);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void BuildDefaultQuests();
    FQuest_Data* FindQuestByID(const FString& QuestID);
    bool CheckObjectivesComplete(const FQuest_Data& Quest) const;
};
