#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur — Tyrannosaurus Rex
 * Apex predator. Massive health pool, devastating damage, wide detection radius.
 * Solitary hunter. Charges when target is within 600cm. Roars to stun nearby prey.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Roar ability — stuns player within RoarRadius for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void ExecuteRoar();

    /** Charge attack — sprint directly at target if within ChargeActivationDistance */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void InitiateCharge();

    /** Stomp — AoE ground slam dealing StompDamage in StompRadius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void ExecuteStomp();

    /** Returns true if TRex is currently in charge state */
    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsCharging() const { return bIsCharging; }

    /** Returns true if TRex roar is on cooldown */
    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsRoarOnCooldown() const { return RoarCooldownRemaining > 0.0f; }

protected:
    /** Radius within which TRex roar stuns targets (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarRadius = 1500.0f;

    /** Duration of roar stun effect (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarStunDuration = 3.0f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown = 15.0f;

    /** Current roar cooldown remaining */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float RoarCooldownRemaining = 0.0f;

    /** Distance at which TRex transitions to charge attack (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float ChargeActivationDistance = 800.0f;

    /** Speed multiplier during charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float ChargeSpeedMultiplier = 1.8f;

    /** Stomp damage (AoE) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompDamage = 120.0f;

    /** Stomp AoE radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompRadius = 400.0f;

    /** Whether TRex is currently charging */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging = false;

    /** Charge duration timer */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float ChargeDurationRemaining = 0.0f;

    /** Max charge duration (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float MaxChargeDuration = 3.0f;

private:
    /** Tick roar cooldown and charge timer */
    void UpdateAbilityCooldowns(float DeltaTime);

    /** Base walk speed stored before charge */
    float BaseWalkSpeed = 0.0f;
};
