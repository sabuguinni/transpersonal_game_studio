// TyrannosaurusRex.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260701_010
// Apex predator subclass of ADinosaurBase

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator — largest land carnivore. Territorial, ambush hunter.
 * Attacks on sight within 2000cm. Pack-hunts juveniles solo.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    // --- Species-specific properties ---

    /** Roar radius — stuns nearby prey (slows movement 50% for 3s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarRadius = 1200.0f;

    /** Roar cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 15.0f;

    /** Bite force multiplier over base damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float BiteForceMult = 2.5f;

    /** Whether the TRex is currently in a charge attack */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Combat")
    bool bIsCharging = false;

    /** Charge speed (cm/s) — faster than normal movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeed = 1400.0f;

    /** Charge distance threshold to trigger charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeActivationDistance = 800.0f;

    // --- Combat actions ---

    /** Perform territorial roar — applies slow debuff to nearby actors */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Begin charge attack toward target location */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void BeginChargeAttack(const FVector& TargetLocation);

    /** End charge attack, return to normal movement */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void EndChargeAttack();

    // --- Overrides ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    /** Timer handle for roar cooldown */
    FTimerHandle RoarCooldownTimer;

    /** Whether roar is available */
    bool bRoarAvailable = true;

    /** Reset roar availability after cooldown */
    void ResetRoarCooldown();

    /** Apply species defaults — called from constructor */
    virtual void ApplySpeciesDefaults() override;
};
