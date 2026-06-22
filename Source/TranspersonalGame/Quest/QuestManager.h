// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260622_003
// UWorldSubsystem managing all active quests, objectives, and NPC quest givers.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed"),
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    Kill        UMETA(DisplayName = "Kill"),
    Collect     UMETA(DisplayName = "Collect"),
    Reach       UMETA(DisplayName = "Reach"),
    Survive     UMETA(DisplayName = "Survive"),
    Escort      UMETA(DisplayName = "Escort"),
    Craft       UMETA(DisplayName = "Craft"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

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
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , Type(EQuest_ObjectiveType::Kill)
        , RequiredCount(1)
        , CurrentCount(0)
        , bCompleted(false)
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
    FString QuestGiverActorLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;  // 0 = no time limit

    FQuest_Definition()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , QuestGiverActorLabel(TEXT(""))
        , Status(EQuest_Status::Inactive)
        , TimeLimit(0.0f)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_SystemStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 TotalQuestsRegistered;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 ActiveQuestCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 CompletedQuestCount;

    UPROPERTY(BlueprintReadOnly, Category = "Quest")
    int32 FailedQuestCount;

    FQuest_SystemStats()
        : TotalQuestsRegistered(0)
        , ActiveQuestCount(0)
        , CompletedQuestCount(0)
        , FailedQuestCount(0)
    {}
};

// ─── UQuestManager ────────────────────────────────────────────────────────────

UCLASS()
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Register a quest definition
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Definition& QuestDef);

    // Activate a quest by ID
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    // Advance objective progress
    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    // Complete a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    // Fail a quest
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // Query
    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_SystemStats GetSystemStats() const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FString> GetActiveQuestIDs() const;

private:
    void RegisterDefaultQuests();
    bool CheckAllObjectivesComplete(const FQuest_Definition& Quest) const;

    UPROPERTY()
    TMap<FString, FQuest_Definition> QuestRegistry;
};
