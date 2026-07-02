#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRex.generated.h"

/**
 * ATRex — Tyrannosaurus Rex species implementation.
 * Apex predator: high health, devastating melee attack, wide detection radius.
 * Behavior: solitary hunter, charges when aggression threshold met.
 * Inherits full state machine from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRex();

    // --- Species-specific properties ---

    /** Roar cooldown in seconds — TRex roars to stun nearby prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown = 12.0f;

    /** Radius of roar stun effect (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarStunRadius = 600.0f;

    /** Stun duration applied to player on roar (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarStunDuration = 2.5f;

    /** Charge speed multiplier — TRex accelerates to lethal sprint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChargeSpeedMultiplier = 1.8f;

    /** Distance at which TRex initiates charge (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChargeInitiationDistance = 800.0f;

    /** Whether TRex is currently charging */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging = false;

    /** Whether roar is on cooldown */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bRoarOnCooldown = false;

    // --- Overrides ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                             AController* EventInstigator, AActor* DamageCauser) override;

    // --- TRex-specific abilities ---

    /** Perform territorial roar — stuns nearby players/prey */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Initiate charge attack toward target location */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void InitiateCharge(const FVector& TargetLocation);

    /** Called when charge completes or is interrupted */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void EndCharge();

protected:
    /** Timer handle for roar cooldown reset */
    FTimerHandle RoarCooldownTimer;

    /** Timer handle for charge duration */
    FTimerHandle ChargeTimer;

    /** Cached charge target */
    FVector ChargeTarget = FVector::ZeroVector;

    /** Apply TRex-specific stats on top of base defaults */
    void ApplyTRexStats();

    /** Update charge movement each tick */
    void UpdateCharge(float DeltaTime);
};
