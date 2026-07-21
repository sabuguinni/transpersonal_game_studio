// TRexDinosaur.h
// Core Systems Programmer #03 — Cycle AUTO_20260702_008
// Tyrannosaurus Rex — apex predator species implementation
// Inherits from ADinosaurBase, overrides stats and behavior

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur
 * Apex predator — largest terrestrial carnivore in the prehistoric world.
 * High health, massive damage, slow patrol speed but devastating charge.
 * Territorial: attacks anything within 1200 units.
 * Roar ability stuns nearby prey for 2 seconds.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "T-Rex Dinosaur"))
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Roar — stuns nearby actors within RoarRadius for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Charge attack — sprint toward target, deal bonus damage on impact */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void InitiateCharge(AActor* Target);

    /** Called when TRex detects prey within detection range */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TRex|Behavior")
    void OnPreyDetected(AActor* Prey);
    virtual void OnPreyDetected_Implementation(AActor* Prey);

protected:
    /** Radius of roar stun effect in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarRadius = 800.0f;

    /** Duration of roar stun in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarStunDuration = 2.0f;

    /** Cooldown between roars in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown = 15.0f;

    /** Charge speed multiplier applied to RunSpeed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float ChargeSpeedMultiplier = 1.8f;

    /** Bonus damage dealt on charge impact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float ChargeBonusDamage = 80.0f;

    /** Detection range for prey in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float PreyDetectionRange = 1200.0f;

    /** Whether TRex is currently charging */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    bool bIsCharging = false;

    /** Whether TRex is currently roaring */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    bool bIsRoaring = false;

private:
    float LastRoarTime = 0.0f;
    FTimerHandle RoarCooldownTimer;
    FTimerHandle ChargeTimer;

    /** Periodic prey detection sweep */
    void ScanForPrey();
    FTimerHandle PreyScanTimer;
};
