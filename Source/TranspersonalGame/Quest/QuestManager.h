// QuestManager.h
// Agent #14 — Quest & Mission Designer
// PROD_CYCLE_AUTO_20260624_009
// Quest system: 3 active quests tied to crowd simulation herds

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "QuestManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

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
    Explore     UMETA(DisplayName = "Explore"),
    Defend      UMETA(DisplayName = "Defend"),
    Gather      UMETA(DisplayName = "Gather"),
    Rescue      UMETA(DisplayName = "Rescue"),
    Track       UMETA(DisplayName = "Track")
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
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Type QuestType = EQuest_Type::Explore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_State QuestState = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius = 400.0f;

    // Reward data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    int32 RewardXP = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Reward")
    FString RewardItemID;
};

// ─── QuestManager ─────────────────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UQuestManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UQuestManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Quest lifecycle ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterQuest(const FQuest_Data& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    // ── Query ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    // ── Proximity check (called each tick by player) ─────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckProximityTriggers(const FVector& PlayerLocation);

    // ── Built-in quest definitions ────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void RegisterDefaultQuests();

private:
    UPROPERTY()
    TMap<FString, FQuest_Data> QuestRegistry;

    void LogQuestEvent(const FString& Event, const FString& QuestID) const;
};
