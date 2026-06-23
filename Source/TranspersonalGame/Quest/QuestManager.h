#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "QuestManager.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ObjectiveType : uint8
{
    ReachLocation   UMETA(DisplayName = "Reach Location"),
    HuntTarget      UMETA(DisplayName = "Hunt Target"),
    CollectResource UMETA(DisplayName = "Collect Resource"),
    DefendLocation  UMETA(DisplayName = "Defend Location"),
    SurviveDuration UMETA(DisplayName = "Survive Duration"),
    EscapeArea      UMETA(DisplayName = "Escape Area")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_Objective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString ObjectiveID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ObjectiveType ObjectiveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TargetRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bIsCompleted;

    FQuest_Objective()
        : ObjectiveID(TEXT(""))
        , Description(TEXT(""))
        , ObjectiveType(EQuest_ObjectiveType::ReachLocation)
        , TargetLocation(FVector::ZeroVector)
        , TargetRadius(300.0f)
        , RequiredCount(1)
        , CurrentCount(0)
        , bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct FQuest_Reward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceMeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 ResourceBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float StaminaBonus;

    FQuest_Reward()
        : ResourceMeat(0)
        , ResourceHide(0)
        , ResourceBone(0)
        , StaminaBonus(0.0f)
    {}
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
    EQuest_Status Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FQuest_Reward Reward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    bool bHasTimeLimit;

    FQuest_Data()
        : QuestID(TEXT(""))
        , QuestTitle(TEXT(""))
        , QuestDescription(TEXT(""))
        , Status(EQuest_Status::Inactive)
        , TimeLimit(0.0f)
        , bHasTimeLimit(false)
    {}
};

// ─── QuestManager Actor ───────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Quest Lifecycle ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void AdvanceObjective(const FString& QuestID, const FString& ObjectiveID, int32 Amount = 1);

    // ── Query ────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    bool IsQuestActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    TArray<FQuest_Data> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    // ── Stampede Integration (from CrowdSimulationManager) ───────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnStampedeTriggered(FVector StampedeOrigin, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnPlayerReachedLocation(FVector PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnDinoKilled(FString DinoSpecies);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void OnResourceCollected(FString ResourceType, int32 Amount);

    // ── Built-in Quests ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Setup")
    void RegisterDefaultQuests();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Quest|Debug")
    void PrintActiveQuests();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Data> QuestRegistry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float QuestTickInterval;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Quest")
    float ElapsedTime;

private:
    void TickQuestTimers(float DeltaTime);
    bool FindQuest(const FString& QuestID, FQuest_Data& OutQuest) const;
    int32 FindQuestIndex(const FString& QuestID) const;
    void CheckObjectiveCompletion(FQuest_Data& Quest);
    void GrantReward(const FQuest_Reward& Reward);

    FQuest_Data BuildQuest_TrackTheHerd();
    FQuest_Data BuildQuest_WaterHoleDefense();
    FQuest_Data BuildQuest_CanyonEscape();
    FQuest_Data BuildQuest_RaptorHunt();
    FQuest_Data BuildQuest_RiverCrossing();
};
