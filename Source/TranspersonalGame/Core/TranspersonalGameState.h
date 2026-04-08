#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "TranspersonalGameState.generated.h"

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Initialization,
    EarlyGame,
    MidGame,
    LateGame,
    EndGame
};

UENUM(BlueprintType)
enum class ETimeOfDay : uint8
{
    Dawn,
    Morning,
    Noon,
    Afternoon,
    Dusk,
    Night,
    Midnight
};

/**
 * Game State for Transpersonal Game
 * Tracks global game state and progression
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API ATranspersonalGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ATranspersonalGameState();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    /** Get current game phase */
    UFUNCTION(BlueprintPure, Category = "Game State")
    EGamePhase GetCurrentGamePhase() const { return CurrentGamePhase; }

    /** Get current time of day */
    UFUNCTION(BlueprintPure, Category = "Game State")
    ETimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    /** Get game time in hours (0-24) */
    UFUNCTION(BlueprintPure, Category = "Game State")
    float GetGameTimeHours() const { return GameTimeHours; }

    /** Get survival time in minutes */
    UFUNCTION(BlueprintPure, Category = "Game State")
    float GetSurvivalTimeMinutes() const { return SurvivalTimeMinutes; }

    /** Get number of creatures alive */
    UFUNCTION(BlueprintPure, Category = "Game State")
    int32 GetCreatureCount() const { return CreatureCount; }

    /** Get player death count */
    UFUNCTION(BlueprintPure, Category = "Game State")
    int32 GetPlayerDeathCount() const { return PlayerDeathCount; }

    /** Set game phase */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetGamePhase(EGamePhase NewPhase);

    /** Add creature to count */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void RegisterCreature();

    /** Remove creature from count */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void UnregisterCreature();

    /** Record player death */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void RecordPlayerDeath();

    /** Check if it's night time */
    UFUNCTION(BlueprintPure, Category = "Game State")
    bool IsNightTime() const;

    /** Check if it's day time */
    UFUNCTION(BlueprintPure, Category = "Game State")
    bool IsDayTime() const;

    /** Get time progression speed multiplier */
    UFUNCTION(BlueprintPure, Category = "Game State")
    float GetTimeProgressionSpeed() const { return TimeProgressionSpeed; }

    /** Set time progression speed */
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void SetTimeProgressionSpeed(float NewSpeed) { TimeProgressionSpeed = FMath::Clamp(NewSpeed, 0.1f, 10.0f); }

protected:
    /** Current game phase */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    EGamePhase CurrentGamePhase = EGamePhase::Initialization;

    /** Current time of day */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    ETimeOfDay CurrentTimeOfDay = ETimeOfDay::Dawn;

    /** Game time in hours (0-24) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    float GameTimeHours = 6.0f; // Start at dawn

    /** Total survival time in minutes */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    float SurvivalTimeMinutes = 0.0f;

    /** Number of creatures currently alive */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    int32 CreatureCount = 0;

    /** Number of times player has died */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Game State")
    int32 PlayerDeathCount = 0;

    /** Time progression speed multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float TimeProgressionSpeed = 1.0f;

    /** Hours per real-time minute */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game Settings")
    float HoursPerMinute = 0.5f; // 48 minutes for full day cycle

private:
    /** Update time of day based on current hours */
    void UpdateTimeOfDay();

    /** Update game phase based on survival time */
    void UpdateGamePhase();

    /** Broadcast time of day change */
    UFUNCTION()
    void OnTimeOfDayChanged();

    /** Broadcast game phase change */
    UFUNCTION()
    void OnGamePhaseChanged();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /** Event dispatchers */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, ETimeOfDay, NewTimeOfDay);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGamePhaseChanged, EGamePhase, NewPhase);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreatureCountChanged, int32, NewCount);

    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnTimeOfDayChanged OnTimeOfDayChangedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnGamePhaseChanged OnGamePhaseChangedDelegate;

    UPROPERTY(BlueprintAssignable, Category = "Game Events")
    FOnCreatureCountChanged OnCreatureCountChangedDelegate;
};