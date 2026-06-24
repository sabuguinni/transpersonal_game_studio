// TRexCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator subclass of ADinosaurBase.
// Adds charge attack, roar fear radius, and territorial behaviour.

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

// ─── T-Rex Behaviour Phase ────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ETRexPhase : uint8
{
    Stalking        UMETA(DisplayName = "Stalking"),       // Slow approach, low aggro
    Charging        UMETA(DisplayName = "Charging"),       // Full sprint charge attack
    Roaring         UMETA(DisplayName = "Roaring"),        // Fear roar — AoE debuff
    Feeding         UMETA(DisplayName = "Feeding"),        // Eating a kill
    Territorial     UMETA(DisplayName = "Territorial"),    // Patrolling home range
};

// ─── T-Rex Charge Data ────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FTRexChargeData
{
    GENERATED_BODY()

    /** Distance at which the charge is triggered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeRange = 1200.0f;

    /** Speed multiplier during charge (applied to base walk speed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeedMultiplier = 2.4f;

    /** Damage dealt on charge impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDamage = 120.0f;

    /** Knockback force applied to target on charge hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float KnockbackForce = 8000.0f;

    /** Cooldown between charges (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeCooldown = 8.0f;

    /** Stamina cost per charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeStaminaCost = 35.0f;
};

// ─── T-Rex Roar Data ─────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FTRexRoarData
{
    GENERATED_BODY()

    /** Radius of the fear roar AoE */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarRadius = 2500.0f;

    /** Fear duration applied to targets (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float FearDuration = 6.0f;

    /** Fear intensity (0-1, affects movement speed penalty) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float FearIntensity = 0.8f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarCooldown = 20.0f;

    /** Stamina cost per roar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarStaminaCost = 20.0f;
};

// ─── ATRexCharacter ───────────────────────────────────────────────────────────
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (DisplayName = "T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Overrides from ADinosaurBase ──────────────────────────────────────────
    virtual void OnStateChanged_Implementation(EDinoState NewState) override;
    virtual void OnAttackPerformed_Implementation(AActor* Target) override;
    virtual void OnDeath_Implementation() override;

public:
    // ── Charge Attack ─────────────────────────────────────────────────────────

    /** Charge configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    FTRexChargeData ChargeData;

    /** Initiate a charge attack toward the current target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void BeginCharge();

    /** Called when the charge connects with a target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void OnChargeImpact(AActor* HitActor);

    /** Is the T-Rex currently charging? */
    UFUNCTION(BlueprintPure, Category = "TRex|Combat")
    bool IsCharging() const { return bIsCharging; }

    // ── Roar ──────────────────────────────────────────────────────────────────

    /** Roar configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    FTRexRoarData RoarData;

    /** Perform a territorial roar — applies fear to all nearby actors */
    UFUNCTION(BlueprintCallable, Category = "TRex|Roar")
    void PerformRoar();

    /** Can the T-Rex roar right now? */
    UFUNCTION(BlueprintPure, Category = "TRex|Roar")
    bool CanRoar() const;

    // ── Territorial ───────────────────────────────────────────────────────────

    /** Centre of this T-Rex's territory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    FVector TerritoryCenter;

    /** Radius of this T-Rex's territory */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    float TerritoryRadius = 5000.0f;

    /** Is the given location within this T-Rex's territory? */
    UFUNCTION(BlueprintPure, Category = "TRex|Territory")
    bool IsInTerritory(const FVector& Location) const;

    // ── Current Phase ─────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    ETRexPhase CurrentPhase = ETRexPhase::Territorial;

    /** Blueprint event fired when the T-Rex phase changes */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TRex|State")
    void OnPhaseChanged(ETRexPhase NewPhase);
    virtual void OnPhaseChanged_Implementation(ETRexPhase NewPhase);

protected:
    // ── Internal state ────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCharging = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State",
        meta = (AllowPrivateAccess = "true"))
    float ChargeTimer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State",
        meta = (AllowPrivateAccess = "true"))
    float RoarTimer = 0.0f;

    /** Set the current phase and fire the OnPhaseChanged event */
    void SetPhase(ETRexPhase NewPhase);

    /** Evaluate whether to charge, roar, or bite based on distance/stamina */
    void EvaluateCombatBehaviour(float DeltaTime);

    /** Apply knockback impulse to a hit actor */
    void ApplyKnockback(AActor* Target, const FVector& Direction);
};
