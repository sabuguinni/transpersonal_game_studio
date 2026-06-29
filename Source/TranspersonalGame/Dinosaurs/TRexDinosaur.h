// TRexDinosaur.h
// Transpersonal Game Studio — Core Systems Programmer #03
// Cycle: PROD_CYCLE_AUTO_20260629_011
// T-Rex species-specific dinosaur — apex predator with ambush and charge behaviours

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

// ─── T-Rex Charge State ───────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECore_TRexChargeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Winding     UMETA(DisplayName = "Winding Up"),
    Charging    UMETA(DisplayName = "Charging"),
    Recovering  UMETA(DisplayName = "Recovering")
};

// ─── T-Rex Sense Mode ─────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECore_TRexSenseMode : uint8
{
    Visual      UMETA(DisplayName = "Visual"),
    Vibration   UMETA(DisplayName = "Ground Vibration"),
    Scent       UMETA(DisplayName = "Scent Trail")
};

// ─── T-Rex Combat Stats ───────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TRexCombatStats
{
    GENERATED_BODY()

    /** Bite force in Newtons (realistic: ~35,000 N) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float BiteForceNewtons = 35000.0f;

    /** Charge speed multiplier over base movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeedMultiplier = 2.2f;

    /** Charge windup duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeWindupSeconds = 1.5f;

    /** Charge duration before recovery */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDurationSeconds = 3.0f;

    /** Charge cooldown before next charge attempt */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeCooldownSeconds = 8.0f;

    /** Stomp radius — knocks back nearby actors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadiusCm = 300.0f;

    /** Stomp damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 40.0f;

    /** Bite damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float BiteDamage = 120.0f;
};

// ─── T-Rex Sensory Stats ──────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TRexSensoryStats
{
    GENERATED_BODY()

    /** Visual detection radius (poor eyesight — movement-based) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float VisualRadiusCm = 2500.0f;

    /** Ground vibration detection radius (footsteps) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float VibrationRadiusCm = 1800.0f;

    /** Scent detection radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float ScentRadiusCm = 3500.0f;

    /** Whether the T-Rex is currently tracking a scent trail */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Senses")
    bool bTrackingScent = false;

    /** Last known scent position */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Senses")
    FVector LastScentPosition = FVector::ZeroVector;
};

/**
 * ATRexDinosaur
 *
 * Tyrannosaurus Rex — apex predator of the prehistoric world.
 * Inherits from ADinosaurBase and adds species-specific:
 *   - Charge attack with windup/charge/recovery phases
 *   - Stomp area-of-effect attack
 *   - Multi-modal sensory system (visual, vibration, scent)
 *   - Territorial roar that intimidates nearby prey
 *   - Realistic hunger-driven hunting behaviour
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    // ─── UE5 Lifecycle ────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── Combat ───────────────────────────────────────────────────────────────

    /** Initiates a charge attack toward the target location */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void InitiateCharge(FVector TargetLocation);

    /** Performs a stomp attack — damages all actors within StompRadius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Performs a bite attack on the current target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformBite();

    /** Territorial roar — intimidates prey within 1500cm, triggers flee response */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void TerritorialRoar();

    // ─── Senses ───────────────────────────────────────────────────────────────

    /** Detects prey using all available sense modes */
    UFUNCTION(BlueprintCallable, Category = "TRex|Senses")
    bool DetectPreyMultiSense(AActor*& OutDetectedPrey);

    /** Updates scent trail tracking */
    UFUNCTION(BlueprintCallable, Category = "TRex|Senses")
    void UpdateScentTracking(float DeltaTime);

    // ─── State Queries ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    ECore_TRexChargeState GetChargeState() const { return ChargeState; }

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsCharging() const { return ChargeState == ECore_TRexChargeState::Charging; }

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool CanCharge() const;

    // ─── Properties ───────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    FCore_TRexCombatStats CombatStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    FCore_TRexSensoryStats SensoryStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    ECore_TRexChargeState ChargeState = ECore_TRexChargeState::Idle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    ECore_TRexSenseMode ActiveSenseMode = ECore_TRexSenseMode::Visual;

    /** Current charge target location */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    FVector ChargeTargetLocation = FVector::ZeroVector;

    /** Time since last charge (for cooldown) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastCharge = 0.0f;

    /** Time remaining in current charge phase */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float ChargePhaseTimer = 0.0f;

    /** Whether the T-Rex has detected a target this frame */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    bool bHasActiveTarget = false;

protected:
    // ─── Internal Charge Logic ────────────────────────────────────────────────
    void TickCharge(float DeltaTime);
    void BeginWindup();
    void BeginChargePhase();
    void BeginRecovery();
    void EndCharge();

    // ─── Internal Sense Logic ─────────────────────────────────────────────────
    bool CheckVisualDetection(AActor*& OutPrey) const;
    bool CheckVibrationDetection(AActor*& OutPrey) const;
    bool CheckScentDetection(AActor*& OutPrey) const;

    /** Cached reference to detected prey */
    UPROPERTY()
    AActor* CurrentPrey = nullptr;

    /** Scent tracking accumulator */
    float ScentTrackingTimer = 0.0f;
};
