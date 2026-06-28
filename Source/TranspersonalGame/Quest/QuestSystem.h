#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestSystem.generated.h"

// ============================================================
// Quest & Mission System — Agent #14 Quest Designer
// Cycle: PROD_CYCLE_AUTO_20260627_013
// ============================================================

// Quest states
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Locked      UMETA(DisplayName = "Locked"),
    Available   UMETA(DisplayName = "Available"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// Quest objective types
UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Hunt        UMETA(DisplayName = "Hunt"),
    Collect     UMETA(DisplayName = "Collect"),
    Explore     UMETA(DisplayName = "Explore"),
    Survive     UMETA(DisplayName = "Survive"),
    Defend      UMETA(DisplayName = "Defend"),
    Craft       UMETA(DisplayName = "Craft"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Track       UMETA(DisplayName = "Track")
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
    EQuest_ObjectiveType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString TargetTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , Type(EQuest_ObjectiveType::Explore)
        , TargetTag(TEXT(""))
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
    {}
};

// Quest reward
USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ItemID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float ExperiencePoints;

    FQuest_Reward()
        : ItemID(TEXT(""))
        , Quantity(0)
        , ExperiencePoints(0.0f)
    {}
};

// Full quest definition
USTRUCT(BlueprintType)
struct FQuest_Definition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    EQuest_State State;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestGiverNPCTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsMainQuest;

    FQuest_Definition()
        : QuestID(TEXT(""))
        , Title(TEXT(""))
        , Description(TEXT(""))
        , State(EQuest_State::Locked)
        , QuestGiverNPCTag(TEXT(""))
        , bIsMainQuest(false)
    {}
};

// ============================================================
// Quest Trigger Actor — placed in world to start quests
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Quest Trigger"))
class TRANSPERSONALGAME_API AQuest_TriggerActor : public AActor
{
    GENERATED_BODY()

public:
    AQuest_TriggerActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestIDToActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bTriggeredOnce;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool IsPlayerInRange() const;

private:
    bool bHasTriggered;
};

// ============================================================
// Quest Manager — singleton-style actor managing all quests
// ============================================================
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Quest Manager"))
class TRANSPERSONALGAME_API AQuest_Manager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_Manager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // All quests in the game
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Definition> AllQuests;

    // Currently active quest IDs
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> ActiveQuestIDs;

    // Completed quest IDs
    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    TArray<FString> CompletedQuestIDs;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool UpdateObjective(const FString& QuestID, const FString& ObjectiveID, int32 Progress);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Definition GetQuestByID(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Definition> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void InitializeDefaultQuests();

private:
    void CheckQuestCompletion(FQuest_Definition& Quest);
    void GrantReward(const FQuest_Reward& Reward);
    int32 FindQuestIndex(const FString& QuestID) const;
};
