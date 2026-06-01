#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Abandoned       UMETA(DisplayName = "Abandoned")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),
    Gather          UMETA(DisplayName = "Gather"),
    Explore         UMETA(DisplayName = "Explore"),
    Craft           UMETA(DisplayName = "Craft"),
    Survive         UMETA(DisplayName = "Survive"),
    Escort          UMETA(DisplayName = "Escort"),
    Defend          UMETA(DisplayName = "Defend")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_Objective()
    {
        Description = TEXT("");
        ObjectiveType = EQuest_Type::Hunt;
        TargetTag = TEXT("");
        RequiredCount = 1;
        CurrentCount = 0;
        bCompleted = false;
        TargetLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString NPCGiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector QuestLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Data()
    {
        QuestID = TEXT("");
        QuestName = TEXT("");
        QuestDescription = TEXT("");
        Status = EQuest_Status::NotStarted;
        QuestType = EQuest_Type::Hunt;
        NPCGiver = TEXT("");
        QuestLocation = FVector::ZeroVector;
        ExperienceReward = 100;
        TimeLimit = 0.0f;
        bHasTimeLimit = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    // Quest Creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntQuest(const FString& QuestID, const FString& TargetSpecies, int32 RequiredKills, const FString& NPCGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherQuest(const FString& QuestID, const FString& ResourceType, int32 RequiredAmount, const FString& NPCGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExploreQuest(const FString& QuestID, const FVector& TargetLocation, float ExploreRadius, const FString& NPCGiver);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateCraftQuest(const FString& QuestID, const FString& ItemToCraft, int32 RequiredAmount, const FString& NPCGiver);

    // Event Handlers
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& Species, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemGathered(const FString& ItemType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnLocationReached(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCrafted(const FString& ItemType, int32 Amount);

    // Quest Discovery
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void SpawnQuestNPCs();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_Data> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float QuestUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float LastUpdateTime;

    // Quest NPCs
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<AActor*> QuestNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<ATriggerBox*> QuestTriggers;

private:
    void UpdateQuestTimers(float DeltaTime);
    void CheckObjectiveCompletion();
    bool IsObjectiveCompleted(const FQuest_Objective& Objective) const;
    void GiveQuestRewards(const FQuest_Data& Quest);
    FQuest_Data* FindQuestByID(const FString& QuestID);
    void LogQuestEvent(const FString& Message);
};