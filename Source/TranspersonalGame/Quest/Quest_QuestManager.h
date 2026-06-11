#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Quest_QuestManager.generated.h"

// Forward declarations
class UQuest_QuestComponent;
class AQuest_QuestGiverNPC;

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    TurnedIn        UMETA(DisplayName = "Turned In")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    KillTarget      UMETA(DisplayName = "Kill Target"),
    CollectItems    UMETA(DisplayName = "Collect Items"),
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    TalkToNPC       UMETA(DisplayName = "Talk to NPC"),
    Survive         UMETA(DisplayName = "Survive"),
    Craft           UMETA(DisplayName = "Craft Item")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_QuestObjective()
    {
        ObjectiveID = "";
        Description = FText::FromString("Default Objective");
        Type = EQuest_ObjectiveType::CollectItems;
        TargetID = "";
        RequiredAmount = 1;
        CurrentAmount = 0;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    FQuest_QuestReward()
    {
        ItemID = "";
        Quantity = 1;
        ExperiencePoints = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestReward> Rewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredLevel;

    FQuest_QuestData()
    {
        QuestID = "";
        Title = FText::FromString("Default Quest");
        Description = FText::FromString("Default quest description");
        Status = EQuest_QuestStatus::NotStarted;
        QuestGiverID = "";
        RequiredLevel = 1;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_QuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_QuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    class UDataTable* QuestDataTable;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool TurnInQuest(const FString& QuestID, class ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData* GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(AQuest_QuestGiverNPC* QuestGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UnregisterQuestGiver(AQuest_QuestGiverNPC* QuestGiver);

protected:
    UPROPERTY()
    TArray<AQuest_QuestGiverNPC*> RegisteredQuestGivers;

    void LoadQuestsFromDataTable();
    void CreateSurvivalQuests();
    void CreateHuntingQuests();
    void CreateExplorationQuests();
    bool CheckQuestPrerequisites(const FQuest_QuestData& Quest, class ATranspersonalCharacter* Player);
    void GiveQuestRewards(const FQuest_QuestData& Quest, class ATranspersonalCharacter* Player);
};