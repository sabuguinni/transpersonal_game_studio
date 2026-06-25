#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestManager.generated.h"

// ============================================================
// Quest & Mission Designer — Agent #14
// QuestManager.h — Core quest system for dinosaur survival game
// Three quests: Track the Herd, Observe Without Disturbing,
//               Survive the Ambush
// ============================================================

UENUM(BlueprintType)
enum class EQuest_Status : uint8
{
    Inactive    UMETA(DisplayName = "Inactive"),
    Active      UMETA(DisplayName = "Active"),
    Completed   UMETA(DisplayName = "Completed"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EQuest_ID : uint8
{
    None                    UMETA(DisplayName = "None"),
    TrackTheHerd            UMETA(DisplayName = "Track The Herd"),
    ObserveWithoutDisturbing UMETA(DisplayName = "Observe Without Disturbing"),
    SurviveTheAmbush        UMETA(DisplayName = "Survive The Ambush")
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
    bool bCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float TriggerRadius = 500.0f;
};

USTRUCT(BlueprintType)
struct FQuest_Data
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_ID QuestID = EQuest_ID::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    EQuest_Status Status = EQuest_Status::Inactive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    TArray<FQuest_Objective> Objectives;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    int32 CurrentObjectiveIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardHungerBonus = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest")
    float RewardStaminaBonus = 0.0f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuestManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Quest Activation ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void ActivateQuest(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CompleteCurrentObjective(EQuest_ID QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void FailQuest(EQuest_ID QuestID);

    // ---- Quest State Queries ----

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    EQuest_Status GetQuestStatus(EQuest_ID QuestID) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    int32 GetActiveQuestCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest")
    FString GetCurrentObjectiveDescription(EQuest_ID QuestID) const;

    // ---- Proximity Check (called each tick) ----

    UFUNCTION(BlueprintCallable, Category = "Quest")
    void CheckPlayerProximityToTriggers(FVector PlayerLocation);

    // ---- Quest Definitions ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Data")
    TArray<FQuest_Data> AllQuests;

    // ---- Config ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Config")
    bool bAutoActivateOnProximity = true;

    // ---- Raptor Ambush Config ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Ambush")
    FVector AmbushZoneCenter = FVector(2500.0f, 1500.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Ambush")
    float AmbushZoneRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Ambush")
    FVector AmbushEscapePoint = FVector(2700.0f, 1200.0f, 400.0f);

    // ---- Trike Observe Config ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Observe")
    FVector ObserveZoneCenter = FVector(3200.0f, 2050.0f, 400.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Observe")
    float ObserveZoneRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Observe")
    float ObserveDisturbanceRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Observe")
    float ObserveRequiredSeconds = 10.0f;

private:
    float ProximityCheckTimer = 0.0f;
    float ObserveAccumulatedTime = 0.0f;

    void InitializeQuestDefinitions();
    FQuest_Data* FindQuestByID(EQuest_ID QuestID);
    const FQuest_Data* FindQuestByIDConst(EQuest_ID QuestID) const;
};
