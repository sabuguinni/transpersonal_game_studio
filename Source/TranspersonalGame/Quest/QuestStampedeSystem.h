#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestStampedeSystem.generated.h"

// ============================================================
// EQuest_StampedeObjective — ordered objectives for stampede quest
// ============================================================
UENUM(BlueprintType)
enum class EQuest_StampedeObjective : uint8
{
    None            UMETA(DisplayName = "None"),
    ObserveHerd     UMETA(DisplayName = "Observe the Herd"),
    DetectThreat    UMETA(DisplayName = "Detect Incoming Predator"),
    ReachHighGround UMETA(DisplayName = "Reach High Ground"),
    SurviveWave     UMETA(DisplayName = "Survive Stampede Wave"),
    Completed       UMETA(DisplayName = "Quest Completed")
};

// ============================================================
// EQuest_StampedeCause — mirrors ECrowd_StampedeCause for quest logic
// ============================================================
UENUM(BlueprintType)
enum class EQuest_StampedeCause : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Predator        UMETA(DisplayName = "Predator Sighting"),
    Fire            UMETA(DisplayName = "Fire"),
    Earthquake      UMETA(DisplayName = "Earthquake"),
    PackLeaderDeath UMETA(DisplayName = "Pack Leader Death")
};

// ============================================================
// FQuest_StampedeRecord — tracks one stampede event for journal
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_StampedeRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Stampede")
    EQuest_StampedeCause Cause = EQuest_StampedeCause::Unknown;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Stampede")
    float PanicPeakLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Stampede")
    bool bPlayerSurvived = false;

    UPROPERTY(BlueprintReadWrite, Category = "Quest|Stampede")
    float TimeToReachSafety = 0.0f;
};

// ============================================================
// AQuest_StampedeManager — orchestrates "Survive the Stampede" quest
// ============================================================
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_StampedeManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_StampedeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ---- Quest State ----
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Stampede")
    EQuest_StampedeObjective CurrentObjective;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Stampede")
    bool bQuestActive;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Stampede")
    bool bQuestCompleted;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Stampede")
    FQuest_StampedeRecord LastStampedeRecord;

    // ---- Configuration ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float HighGroundMinZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float HerdObserveRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    float StampedeWaveSurviveTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Stampede")
    FVector HerdObserveLocation;

    // ---- Quest Control ----
    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void StartQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void AdvanceObjective(EQuest_StampedeObjective NewObjective);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void OnStampedeTriggered(EQuest_StampedeCause Cause, FVector Location, float PanicLevel);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void OnStampedeEnded(bool bPlayerSurvived);

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    void CheckPlayerOnHighGround();

    UFUNCTION(BlueprintCallable, Category = "Quest|Stampede")
    bool IsPlayerNearHerd() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    FString GetObjectiveText() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Stampede")
    float GetQuestProgress() const;

    // ---- Delegates ----
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectiveChanged, EQuest_StampedeObjective, NewObjective);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Stampede")
    FOnObjectiveChanged OnObjectiveChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, FQuest_StampedeRecord, Record);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Stampede")
    FOnQuestCompleted OnQuestCompleted;

private:
    float SurviveTimer;
    bool bOnHighGround;

    void CompleteQuest();
    void FailQuest();
};
