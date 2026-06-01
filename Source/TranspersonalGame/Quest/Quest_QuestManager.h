#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "Quest_QuestManager.generated.h"

UENUM(BlueprintType)
enum class EQuest_QuestType : uint8
{
    Hunt_Raptor         UMETA(DisplayName = "Hunt Raptors"),
    Hunt_TRex           UMETA(DisplayName = "Hunt T-Rex"),
    Gather_Resources    UMETA(DisplayName = "Gather Resources"),
    Explore_Territory   UMETA(DisplayName = "Explore Territory"),
    Craft_Tools         UMETA(DisplayName = "Craft Tools"),
    Defend_Camp         UMETA(DisplayName = "Defend Camp"),
    Rescue_Survivor     UMETA(DisplayName = "Rescue Survivor"),
    Track_Migration     UMETA(DisplayName = "Track Migration")
};

UENUM(BlueprintType)
enum class EQuest_QuestStatus : uint8
{
    NotStarted          UMETA(DisplayName = "Not Started"),
    Active              UMETA(DisplayName = "Active"),
    Completed           UMETA(DisplayName = "Completed"),
    Failed              UMETA(DisplayName = "Failed"),
    Abandoned           UMETA(DisplayName = "Abandoned")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_QuestObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    FQuest_QuestObjective()
    {
        ObjectiveDescription = TEXT("Default Objective");
        RequiredCount = 1;
        CurrentCount = 0;
        bIsCompleted = false;
        TargetLocation = FVector::ZeroVector;
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
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_QuestData()
    {
        QuestID = TEXT("DefaultQuest");
        QuestName = TEXT("Default Quest");
        QuestDescription = TEXT("Default quest description");
        QuestType = EQuest_QuestType::Gather_Resources;
        QuestStatus = EQuest_QuestStatus::NotStarted;
        ExperienceReward = 100;
        TimeLimit = 0.0f;
        bHasTimeLimit = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_QuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_QuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool AbandonQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void UpdateQuestObjective(const FString& QuestID, int32 ObjectiveIndex, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_QuestData> GetCompletedQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    FQuest_QuestData GetQuestByID(const FString& QuestID) const;

    // Quest Creation
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHuntQuest(const FString& TargetSpecies, int32 RequiredKills);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateGatherQuest(const FString& ResourceType, int32 RequiredAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateExploreQuest(const FVector& TargetLocation, float ExploreRadius);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateCraftQuest(const FString& ItemToCraft, int32 RequiredAmount);

    // Quest Triggers
    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnDinosaurKilled(const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnResourceGathered(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnLocationReached(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void OnItemCrafted(const FString& ItemType, int32 Amount);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> AllQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_QuestData> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    bool bAutoSaveProgress;

private:
    void InitializeDefaultQuests();
    void CheckQuestCompletion(FQuest_QuestData& Quest);
    void GiveQuestRewards(const FQuest_QuestData& Quest);
    FString GenerateQuestID();
    
    int32 QuestIDCounter;
};