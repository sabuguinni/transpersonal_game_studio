// QuestDefenseObjective.h
// Agent #14 — Quest & Mission Designer
// Camp defense quest: protect the camp from predator attacks

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "QuestDefenseObjective.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EQuest_DefensePhase : uint8
{
    NotStarted      UMETA(DisplayName = "Not Started"),
    Preparation     UMETA(DisplayName = "Preparation — gather weapons"),
    FirstWave       UMETA(DisplayName = "First Wave — raptors at perimeter"),
    BetweenWaves    UMETA(DisplayName = "Between Waves — repair defenses"),
    SecondWave      UMETA(DisplayName = "Second Wave — TRex approach"),
    FinalStand      UMETA(DisplayName = "Final Stand — drive off alpha"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed — camp destroyed")
};

UENUM(BlueprintType)
enum class EQuest_DefenseFailReason : uint8
{
    None            UMETA(DisplayName = "None"),
    CampDestroyed   UMETA(DisplayName = "Camp Destroyed"),
    PlayerDied      UMETA(DisplayName = "Player Died"),
    AllNPCsDead     UMETA(DisplayName = "All NPCs Dead"),
    TimeExpired     UMETA(DisplayName = "Time Expired")
};

UENUM(BlueprintType)
enum class EQuest_PredatorType : uint8
{
    Raptor          UMETA(DisplayName = "Raptor"),
    Dilophosaurus   UMETA(DisplayName = "Dilophosaurus"),
    Carnotaurus     UMETA(DisplayName = "Carnotaurus"),
    TRex            UMETA(DisplayName = "T-Rex"),
    AlphaPredator   UMETA(DisplayName = "Alpha Predator")
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FQuest_PredatorWave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    EQuest_PredatorType PredatorType = EQuest_PredatorType::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    int32 PredatorCount = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float WaveDelaySeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    FVector SpawnDirectionBias = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    bool bIsAlphaWave = false;
};

USTRUCT(BlueprintType)
struct FQuest_CampDefensePoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    FString PointLabel = TEXT("Defense Point");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float CurrentHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    bool bIsDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float DefenseRadius = 300.0f;
};

// ─── Delegates ────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDefensePhaseChanged, EQuest_DefensePhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefenseWaveStarted, int32, WaveIndex, EQuest_PredatorType, PredatorType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDefensePointDamaged, float, RemainingHealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDefenseQuestCompleted, bool, bSuccess, EQuest_DefenseFailReason, Reason);

// ─── Component ────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Quest), meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UQuestDefenseObjective : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuestDefenseObjective();

    // ── Quest control ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Quest|Defense")
    void StartDefenseQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Defense")
    void TriggerNextWave();

    UFUNCTION(BlueprintCallable, Category = "Quest|Defense")
    void DamageCampPoint(int32 PointIndex, float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Defense")
    void RepairCampPoint(int32 PointIndex, float RepairAmount);

    UFUNCTION(BlueprintCallable, Category = "Quest|Defense")
    void OnWaveCleared();

    // ── Queries ───────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    EQuest_DefensePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    int32 GetCurrentWaveIndex() const { return CurrentWaveIndex; }

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    float GetCampIntegrityPercent() const;

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    bool IsQuestActive() const { return bQuestActive; }

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    float GetTimeUntilNextWave() const { return TimeUntilNextWave; }

    UFUNCTION(BlueprintPure, Category = "Quest|Defense")
    FQuest_PredatorWave GetCurrentWaveData() const;

    // ── Delegates ─────────────────────────────────────────────────────────────

    UPROPERTY(BlueprintAssignable, Category = "Quest|Defense")
    FOnDefensePhaseChanged OnDefensePhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Defense")
    FOnDefenseWaveStarted OnDefenseWaveStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Defense")
    FOnDefensePointDamaged OnDefensePointDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Quest|Defense")
    FOnDefenseQuestCompleted OnDefenseQuestCompleted;

    // ── Config ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    TArray<FQuest_PredatorWave> WaveDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    TArray<FQuest_CampDefensePoint> CampDefensePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float BetweenWavesDuration = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Defense")
    float PreparationDuration = 30.0f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void SetupDefaultWaves();
    void SetupDefaultCampPoints();
    void AdvancePhase(EQuest_DefensePhase NewPhase);
    void CompleteQuest(bool bSuccess, EQuest_DefenseFailReason Reason);
    void UpdateTimers(float DeltaTime);
    bool CheckCampDestroyed() const;

    UPROPERTY()
    EQuest_DefensePhase CurrentPhase;

    UPROPERTY()
    EQuest_DefenseFailReason FailReason;

    UPROPERTY()
    bool bQuestActive;

    UPROPERTY()
    bool bQuestCompleted;

    UPROPERTY()
    int32 CurrentWaveIndex;

    UPROPERTY()
    float TimeUntilNextWave;

    UPROPERTY()
    float PreparationTimeRemaining;
};
