#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_QuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted    UMETA(DisplayName = "Not Started"),
    Active        UMETA(DisplayName = "Active"),
    Completed     UMETA(DisplayName = "Completed"),
    Failed        UMETA(DisplayName = "Failed"),
    Abandoned     UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    Hunt          UMETA(DisplayName = "Hunt"),
    Gather        UMETA(DisplayName = "Gather"),
    Explore       UMETA(DisplayName = "Explore"),
    Craft         UMETA(DisplayName = "Craft"),
    Rescue        UMETA(DisplayName = "Rescue"),
    Defend        UMETA(DisplayName = "Defend")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredProgress;

    FQuest_QuestObjective()
    {
        Description = TEXT("Default Objective");
        bIsCompleted = false;
        CurrentProgress = 0;
        RequiredProgress = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_QuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_QuestObjective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverID;

    FQuest_QuestData()
    {
        QuestID = TEXT("QUEST_DEFAULT");
        QuestName = TEXT("Default Quest");
        QuestDescription = TEXT("A basic survival quest");
        QuestType = EQuest_QuestType::Gather;
        QuestStatus = EQuest_QuestStatus::NotStarted;
        ExperienceReward = 100;
        QuestGiverID = TEXT("NPC_DEFAULT");
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
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool UpdateQuestObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestData(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CanAcceptQuest(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void RegisterQuestGiver(const FString& QuestGiverID, class AQuest_QuestGiverNPC* QuestGiver);

private:
    UPROPERTY()
    TMap<FString, class AQuest_QuestGiverNPC*> QuestGivers;

    void CreateHuntQuests();
    void CreateGatherQuests();
    void CreateExploreQuests();
    void CreateCraftQuests();
    void CreateRescueQuests();
};