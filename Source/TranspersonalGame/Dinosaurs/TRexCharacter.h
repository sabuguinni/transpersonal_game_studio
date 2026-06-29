// TRexCharacter.h
// Transpersonal Game Studio — Core Systems Programmer (Agent #3)
// Cycle: PROD_CYCLE_AUTO_20260629_009
// Tyrannosaurus Rex — apex predator concrete subclass of DinosaurBase

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Concrete implementation of the Tyrannosaurus Rex.
 * Apex predator with high health, devastating attack damage, and territorial aggression.
 * Inherits all survival loops (hunger drain, stamina regen) from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Species-Specific Overrides ---

    /** TRex roar — alerts nearby prey, triggers fear response on player */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void PerformRoar();

    /** Stomp attack — area-of-effect ground slam dealing 80% of AttackDamage in radius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStompAttack();

    /** Called when TRex detects prey — initiates charge sequence */
    virtual void OnAlerted() override;

    /** Called when TRex health drops below 30% — enters enraged state */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void EnterEnragedState();

    // --- TRex-Specific Properties ---

    /** Radius of stomp AoE attack in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 350.0f;

    /** Stomp damage multiplier relative to base AttackDamage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamageMultiplier = 0.8f;

    /** Roar radius — prey within this range receive fear debuff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarRadius = 1500.0f;

    /** Fear intensity applied to player on roar (0-1 scale) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarFearIntensity = 0.75f;

    /** Speed multiplier when enraged (health < 30%) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float EnragedSpeedMultiplier = 1.4f;

    /** Is TRex currently in enraged state? */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsEnraged = false;

    /** Cooldown between roar events in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 12.0f;

    /** Time since last roar */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastRoar = 0.0f;

    /** Charge speed when sprinting toward prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChargeSpeed = 1100.0f;

protected:
    /** Timer handle for roar cooldown tracking */
    FTimerHandle RoarCooldownTimer;

    /** Whether roar is currently on cooldown */
    bool bRoarOnCooldown = false;

    /** Reset roar cooldown */
    void ResetRoarCooldown();

    /** Check enrage threshold each tick */
    void CheckEnrageThreshold();
};
