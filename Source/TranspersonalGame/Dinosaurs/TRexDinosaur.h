#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur
 * Apex predator — Tyrannosaurus Rex subclass of ADinosaurBase.
 * Stats: MaxHealth=5000, AttackDamage=400, MoveSpeed=800, Mass=12000, DetectionRadius=3500.
 * Solitary hunter with massive territory radius and devastating bite attack.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    /** Roar ability — stuns nearby prey for 2 seconds, triggers Aggressive state */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Stomp attack — area damage in 600-unit radius, used when prey is directly underfoot */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStomp();

    /** Override: T-Rex charges at target when in Hunting state beyond 1500 units */
    virtual void UpdateBehavior() override;

protected:
    virtual void BeginPlay() override;

    /** Radius of stomp area damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 600.f;

    /** Stomp damage (separate from bite damage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 200.f;

    /** Roar stun duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarStunDuration = 2.0f;

    /** Charge speed multiplier when hunting at distance */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChargeSpeedMultiplier = 1.8f;

    /** Minimum distance to target before switching from charge to bite */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSwitchDistance = 400.f;

private:
    /** Timer handle for roar cooldown */
    FTimerHandle RoarCooldownHandle;

    /** Whether roar is currently on cooldown */
    bool bRoarOnCooldown = false;

    /** Roar cooldown duration */
    float RoarCooldownDuration = 15.0f;

    void ResetRoarCooldown();
};
