// TRexCharacter.h — Tyrannosaurus Rex — apex predator species
// Inherits from ADinosaurBase. Species-specific stats, charge attack, roar ability.
// Agent #03 — Core Systems Programmer — PROD_CYCLE_AUTO_20260625_002

#pragma once

#include "CoreMinimal.h"
#include "Core/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Tyrannosaurus Rex — the apex predator of the prehistoric world.
 * Stats: Health=500, RunSpeed=900, AttackDamage=120, SenseRadius=3000
 * Behaviour: Solitary hunter, charges prey, roars to stun nearby animals.
 * Inherits full metabolism, damage, death/decay from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "DinosaurAI")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // === OVERRIDES ===
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === CHARGE ATTACK ===
    /** Initiates a charge towards the target — covers ground fast, high damage on impact */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StartCharge(AActor* Target);

    /** Called when charge impact occurs — deals bonus damage and knockback */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void OnChargeImpact(AActor* HitActor);

    /** Stop charge (wall hit, target dead, stamina depleted) */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StopCharge();

    // === ROAR ===
    /** Intimidation roar — stuns nearby prey for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Blueprint event — play roar animation + sound */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Abilities")
    void OnRoar();

    /** Blueprint event — play charge animation */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Combat")
    void OnChargeStart();

    /** Blueprint event — play charge impact animation */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Combat")
    void OnChargeImpactEvent(AActor* HitActor);

    // === STATS (species-specific) ===
    /** Charge speed multiplier over base movement speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float ChargeSpeedMultiplier = 2.2f;

    /** Charge impact bonus damage (added to base AttackDamage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float ChargeImpactBonusDamage = 80.0f;

    /** Roar stun duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarStunDuration = 3.0f;

    /** Roar radius — all prey within this range are stunned */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarRadius = 1500.0f;

    /** Roar cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarCooldown = 20.0f;

    /** Charge stamina cost */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float ChargeStaminaCost = 40.0f;

    /** Minimum distance to target before charge triggers */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float ChargeActivationDistance = 800.0f;

    /** Is currently charging */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging = false;

    /** Time since last roar */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastRoar = 0.0f;

    /** Current charge target */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    TObjectPtr<AActor> ChargeTarget = nullptr;

private:
    /** Charge duration timer */
    FTimerHandle ChargeTimerHandle;

    /** Roar cooldown timer */
    FTimerHandle RoarCooldownHandle;

    /** Internal: apply charge movement each tick */
    void TickCharge(float DeltaTime);

    /** Internal: check if roar is off cooldown */
    bool CanRoar() const;

    /** Internal: find all stun targets within RoarRadius */
    void ApplyRoarStun();
};
