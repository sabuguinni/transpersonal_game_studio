// TyrannosaurusRex.h — Tyrannosaurus Rex species implementation
// Agent #03 — Core Systems Programmer — Cycle AUTO_20260628_010
#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator. Solitary, territorial, ambush hunter.
 * Stats: 500 HP, 25 damage/hit, 400 walk / 550 run speed.
 * Behavior: patrols large territory, charges on sight, roars to stun prey.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Tyrannosaurus Rex"))
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Roar ability ─────────────────────────────────────────────────────────
    /** Roar that briefly stuns nearby prey (radius 1200 cm, stun 2s) */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformRoar();

    /** Radius of the roar stun effect in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float RoarRadius = 1200.0f;

    /** Duration of roar stun on prey (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float RoarStunDuration = 2.0f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float RoarCooldown = 20.0f;

    // ── Charge attack ────────────────────────────────────────────────────────
    /** Initiates a charge attack toward target location */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void BeginCharge(FVector TargetLocation);

    /** Speed multiplier during charge (applied on top of run speed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float ChargeSpeedMultiplier = 1.5f;

    /** Duration of charge in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float ChargeDuration = 2.5f;

    // ── Territory ────────────────────────────────────────────────────────────
    /** Radius of TRex territory in cm (default 8000 = 80m) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Territory")
    float TerritoryRadius = 8000.0f;

    /** Home base location (set on BeginPlay to spawn location) */
    UPROPERTY(BlueprintReadOnly, Category="TRex|Territory")
    FVector HomeLocation;

protected:
    /** Whether TRex is currently charging */
    UPROPERTY(BlueprintReadOnly, Category="TRex|State")
    bool bIsCharging = false;

    /** Time remaining in current charge */
    float ChargeTimeRemaining = 0.0f;

    /** Timestamp of last roar */
    float LastRoarTime = -999.0f;

    /** Charge target direction (normalised) */
    FVector ChargeDirection = FVector::ZeroVector;
};
