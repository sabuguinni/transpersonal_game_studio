#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ── Quest state enum ──────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EQuest_State : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

// ── Quest objective struct ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount = 1;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Quest")
    bool bCompleted = false;
};

// ── Quest data struct ─────────────────────────────────────────────────────
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
    EQuest_State State = EQuest_State::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardFood = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardReputation = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString GiverActorLabel;
};

// ── QuestManager actor ────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest registry ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Registry")
    TArray<FQuest_Data> AllQuests;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Registry")
    TArray<FString> ActiveQuestIDs;

    // ── Core API ──────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool CompleteObjective(const FString& QuestID, const FString& ObjectiveID, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    FQuest_Data GetQuestData(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    EQuest_State GetQuestState(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    // ── Crowd integration ─────────────────────────────────────────────────
    /** Called when crowd flee event fires — fails "Protect the Camp" if triggered */
    UFUNCTION(BlueprintCallable, Category = "Quest|Crowd")
    void OnCrowdFleeEvent(float FearLevel);

    /** Called when player enters a quest trigger volume */
    UFUNCTION(BlueprintCallable, Category = "Quest|Trigger")
    void OnQuestTriggerEntered(const FString& TriggerID);

    // ── Survival quest helpers ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnDinosaurKilled(const FString& DinoSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnResourceCollected(const FString& ResourceType, int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Survival")
    void OnPlayerReachedLocation(const FString& LocationTag);

private:
    void RegisterDefaultQuests();
    void CheckQuestCompletion(FQuest_Data& Quest);
    int32 FindQuestIndex(const FString& QuestID) const;

    UPROPERTY()
    float TimeSinceLastTick = 0.f;

    static constexpr float QuestTickInterval = 0.5f;
};
