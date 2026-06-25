#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestSystemManager.generated.h"

// ============================================================
// Quest System Manager — Agent #14 Quest & Mission Designer
// PROD_CYCLE_AUTO_20260625_004
// ============================================================

UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Track       UMETA(DisplayName = "Track"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Survive     UMETA(DisplayName = "Survive"),
    Gather      UMETA(DisplayName = "Gather"),
    Explore     UMETA(DisplayName = "Explore")
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
    FVector WorldLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , WorldLocation(FVector::ZeroVector)
        , bIsCompleted(false)
        , RequiredCount(1)
        , CurrentCount(0)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Definition
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
    EQuest_State QuestState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestType(EQuest_Type::Explore)
        , QuestState(EQuest_State::Inactive)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuestSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestSystemManager();

    // Active quests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> ActiveQuests;

    // Completed quests
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    // Max concurrent active quests
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 MaxActiveQuests;

    // Activate a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    // Complete an objective within a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    // Fail a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    // Check if all objectives are done → auto-complete quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CheckQuestCompletion(const FString& QuestID);

    // Get quest state
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    // Register a new quest definition
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    // Initialize default quests (Track Herd, Rescue Hunters, Stampede Survival)
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest")
    void InitializeDefaultQuests();

    // Tick for time-limited quests
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
    virtual void BeginPlay() override;

private:
    // Find quest index by ID
    int32 FindQuestIndex(const FString& QuestID) const;

    // Elapsed time tracker for time-limited quests
    TMap<FString, float> QuestElapsedTimes;
};
