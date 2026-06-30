// TRex.h
// Core Systems Programmer #03 — Cycle AUTO_20260630_004
// Tyrannosaurus Rex — apex predator, solitary, ambush hunter
// Inherits from ADinosaurBase with species-specific stats and behaviors

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRex.generated.h"

/**
 * ATRex — Tyrannosaurus Rex
 *
 * Apex predator of the Cretaceous. Solitary, territorial, ambush hunter.
 * Extremely high health and damage. Slow turn rate but devastating charge.
 * Will investigate sounds and smells before attacking — gives player warning.
 *
 * Stats:
 *   Health: 1000 (tank)
 *   AttackDamage: 150 per bite
 *   MoveSpeed: 550 (fast for its size)
 *   DetectionRadius: 2500 (excellent smell/hearing)
 *   TerritoryRadius: 5000 (huge territory)
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Tyrannosaurus Rex"))
class TRANSPERSONALGAME_API ATRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRex();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Roar System ──────────────────────────────────────────────────────────

    /** Trigger roar — stuns nearby prey, alerts pack territory */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Radius in which roar causes fear/stun on other creatures */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarRadius = 1500.0f;

    /** Roar cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 20.0f;

    // ── Charge Attack ────────────────────────────────────────────────────────

    /** Initiate charge — massive speed burst toward target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void InitiateCharge(AActor* Target);

    /** Speed multiplier during charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeedMultiplier = 2.5f;

    /** Charge duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDuration = 3.0f;

    /** Charge damage on impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDamage = 200.0f;

    // ── Sensory System ───────────────────────────────────────────────────────

    /** T-Rex has poor eyesight — movement-based detection */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    bool bMovementBasedVision = true;

    /** Distance at which T-Rex can smell blood */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Senses")
    float BloodSmellRadius = 3000.0f;

    // ── State Queries ────────────────────────────────────────────────────────

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsCharging() const { return bIsCharging; }

    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsRoaring() const { return bIsRoaring; }

protected:
    /** Called when health drops below 30% — enrages T-Rex */
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

private:
    bool bIsCharging = false;
    bool bIsRoaring = false;
    float LastRoarTime = -999.0f;
    float ChargeTimer = 0.0f;
    AActor* ChargeTarget = nullptr;

    FTimerHandle RoarTimerHandle;
    FTimerHandle ChargeTimerHandle;

    void EndCharge();
    void EndRoar();
    void CheckForBloodSmell();
};
