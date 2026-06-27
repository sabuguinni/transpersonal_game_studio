#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// Quest state enum
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive        UMETA(DisplayName = "Inactive"),
    Available       UMETA(DisplayName = "Available"),
    Active          UMETA(DisplayName = "Active"),
    ObjectiveComplete UMETA(DisplayName = "Objective Complete"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed")
};

// Quest type enum
UENUM(BlueprintType)
enum class EQuest_Type : uint8
{
    Hunt            UMETA(DisplayName = "Hunt"),
    Gather          UMETA(DisplayName = "Gather"),
    Explore         UMETA(DisplayName = "Explore"),
    Defend          UMETA(DisplayName = "Defend"),
    Rescue          UMETA(DisplayName = "Rescue"),
    Observe         UMETA(DisplayName = "Observe"),
    Craft           UMETA(DisplayName = "Craft"),
    Track           UMETA(DisplayName = "Track")
};

// Single quest objective
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsComplete;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector ObjectiveLocation;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsComplete(false)
        , ObjectiveLocation(FVector::ZeroVector)
    {}
};

// Quest reward data
USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceStones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceSticks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceLeaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float HealthBonus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString UnlockedRecipeID;

    FQuest_Reward()
        : ResourceStones(0)
        , ResourceSticks(0)
        , ResourceLeaves(0)
        , HealthBonus(0.0f)
        , UnlockedRecipeID(TEXT(""))
    {}
};

// Full quest data structure
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
    bool bIsMainQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestName(TEXT(""))
        , Description(TEXT(""))
        , QuestType(EQuest_Type::Explore)
        , QuestState(EQuest_State::Inactive)
        , QuestGiverNPCID(TEXT(""))
        , bIsMainQuest(false)
        , TimeLimit(0.0f)
    {}
};

/**
 * AQuest_Manager — Central quest management actor for the prehistoric survival game.
 * Tracks all active, available, and completed quests.
 * Integrates with crowd behavior zones from Agent #13.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_Manager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_Manager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === QUEST REGISTRY ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> CompletedQuestIDs;

    // === QUEST OPERATIONS ===

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool StartQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjectiveProgress(const FString& QuestID, const FString& ObjectiveID, int32 ProgressAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsQuestCompleted(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // === QUEST INITIALIZATION ===

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeDefaultQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    // === CROWD INTEGRATION ===
    // Called by crowd behavior zones (Agent #13) to trigger quest events

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnPlayerEnterWateringHole(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnPlayerEnterNestingGround(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnFleeingAnimalsDetected(const FVector& FleeOrigin);

    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnRaptorAmbushTriggered(const FVector& AmbushLocation);

    // === EVENTS ===

    UPROPERTY(BlueprintAssignable, Category = "Quest|Events")
    FTimerHandle QuestTimerHandle;

private:
    void CheckQuestTimeLimits(float DeltaTime);
    FQuest_Data* FindQuestByID(const FString& QuestID);
    void BuildDefaultQuestLibrary();
};
