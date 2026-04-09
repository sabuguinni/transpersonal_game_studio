#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "QuestManager.generated.h"

class UQuestObjective;
class UQuestInstance;
class AQuestGiver;

UENUM(BlueprintType)
enum class EQuestState : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Turned_In       UMETA(DisplayName = "Turned In")
};

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Main            UMETA(DisplayName = "Main Story"),
    Side            UMETA(DisplayName = "Side Quest"),
    Discovery       UMETA(DisplayName = "Discovery"),
    Survival        UMETA(DisplayName = "Survival"),
    Collection      UMETA(DisplayName = "Collection"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Domestication   UMETA(DisplayName = "Domestication"),
    Crafting        UMETA(DisplayName = "Crafting")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FText QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuestType QuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FGameplayTagContainer QuestTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FString> ObjectiveIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ExperienceReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestDifficulty;

    FQuestData()
    {
        QuestType = EQuestType::Side;
        ExperienceReward = 100;
        bIsMainQuest = false;
        QuestDifficulty = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuestStateChanged, const FString&, QuestID, EQuestState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestStarted, const FString&, QuestID);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Quest Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool TurnInQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuestState GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetCompletedQuests() const;

    // Objective Management
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& ObjectiveID, float Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    float GetObjectiveProgress(const FString& ObjectiveID) const;

    // Quest Discovery
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckQuestAvailability();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CanStartQuest(const FString& QuestID) const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStateChanged OnQuestStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestCompleted OnQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnQuestStarted OnQuestStarted;

    // Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    class UDataTable* QuestDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Data")
    class UDataTable* ObjectiveDataTable;

private:
    UPROPERTY()
    TMap<FString, EQuestState> QuestStates;

    UPROPERTY()
    TMap<FString, class UQuestInstance*> ActiveQuestInstances;

    UPROPERTY()
    TMap<FString, float> ObjectiveProgress;

    void InitializeQuestSystem();
    void LoadQuestData();
    bool ArePrerequisitesMet(const FQuestData& QuestData) const;
    void NotifyQuestStateChange(const FString& QuestID, EQuestState NewState);
};