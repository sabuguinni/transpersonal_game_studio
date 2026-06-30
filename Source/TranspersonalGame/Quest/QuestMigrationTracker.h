#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuestMigrationTracker.generated.h"

// ============================================================
// EQuest_MigrationPhase — stages of the migration quest
// ============================================================
UENUM(BlueprintType)
enum class EQuest_MigrationPhase : uint8
{
    NotStarted       UMETA(DisplayName = "Not Started"),
    LocateHerd       UMETA(DisplayName = "Locate the Herd"),
    FollowHerd       UMETA(DisplayName = "Follow the Herd"),
    ReachWaterSource UMETA(DisplayName = "Reach Water Source"),
    SurviveStampede  UMETA(DisplayName = "Survive Stampede"),
    Completed        UMETA(DisplayName = "Completed"),
    Failed           UMETA(DisplayName = "Failed")
};

// ============================================================
// EQuest_MigrationFailReason
// ============================================================
UENUM(BlueprintType)
enum class EQuest_MigrationFailReason : uint8
{
    None             UMETA(DisplayName = "None"),
    PlayerDied       UMETA(DisplayName = "Player Died"),
    HerdLost         UMETA(DisplayName = "Lost the Herd"),
    TimeExpired      UMETA(DisplayName = "Time Expired"),
    TrampleDeath     UMETA(DisplayName = "Trampled by Stampede")
};

// ============================================================
// FQuest_MigrationWaypoint — a single waypoint along the route
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_MigrationWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float AcceptanceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    FString WaypointLabel = TEXT("Waypoint");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    bool bTriggerStampedeOnArrive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    bool bIsWaterSource = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    bool bReached = false;
};

// ============================================================
// FQuest_MigrationState — runtime state snapshot
// ============================================================
USTRUCT(BlueprintType)
struct FQuest_MigrationState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    EQuest_MigrationPhase CurrentPhase = EQuest_MigrationPhase::NotStarted;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    float ElapsedTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    float TimeLimit = 600.0f;   // 10 minutes

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    bool bStampedeActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    EQuest_MigrationFailReason FailReason = EQuest_MigrationFailReason::None;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    float DistanceToNextWaypoint = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    int32 WaypointsReached = 0;
};

// ============================================================
// AQuest_MigrationTracker — actor placed in level to manage
// the "Follow the Herd" quest chain
// ============================================================
UCLASS(Blueprintable, BlueprintType, meta = (DisplayName = "Quest Migration Tracker"))
class TRANSPERSONALGAME_API AQuest_MigrationTracker : public AActor
{
    GENERATED_BODY()

public:
    AQuest_MigrationTracker();

    // ── Configuration ─────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    TArray<FQuest_MigrationWaypoint> MigrationWaypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float HerdProximityRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float StampedeWarningTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float StampedeDangerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest|Migration")
    float StampedeDamagePerSecond = 35.0f;

    // ── Runtime State ─────────────────────────────────────────
    UPROPERTY(BlueprintReadOnly, Category = "Quest|Migration")
    FQuest_MigrationState MigrationState;

    // ── Public API ────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Quest|Migration")
    void StartMigrationQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Migration")
    void AdvanceToNextWaypoint();

    UFUNCTION(BlueprintCallable, Category = "Quest|Migration")
    void TriggerStampedeEvent();

    UFUNCTION(BlueprintCallable, Category = "Quest|Migration")
    void CompleteQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest|Migration")
    void FailQuest(EQuest_MigrationFailReason Reason);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Migration")
    FQuest_MigrationWaypoint GetCurrentWaypoint() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Migration")
    float GetProgressPercent() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Quest|Migration")
    bool IsPlayerNearHerd(const FVector& PlayerLocation) const;

    UFUNCTION(CallInEditor, Category = "Quest|Migration")
    void DEBUG_StartQuestInEditor();

    // ── Delegates ─────────────────────────────────────────────
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMigrationPhaseChanged, EQuest_MigrationPhase, NewPhase);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Migration")
    FOnMigrationPhaseChanged OnMigrationPhaseChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStampedeTriggered);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Migration")
    FOnStampedeTriggered OnStampedeTriggered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuestCompleted, float, ElapsedTime);
    UPROPERTY(BlueprintAssignable, Category = "Quest|Migration")
    FOnQuestCompleted OnQuestCompleted;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void UpdateWaypointProgress(float DeltaTime);
    void UpdateStampedePhase(float DeltaTime);
    void CheckTimeLimit(float DeltaTime);
    void SetPhase(EQuest_MigrationPhase NewPhase);

    float StampedeElapsed = 0.0f;
    bool bQuestActive = false;
};
