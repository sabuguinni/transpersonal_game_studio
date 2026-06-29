// TRexCharacter.h
// Core Systems Programmer #03 — Cycle AUTO_20260629_004
// Tyrannosaurus Rex — apex predator species implementation

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Tyrannosaurus Rex — the apex predator of the prehistoric world.
 * Inherits all base dinosaur behavior from ADinosaurBase.
 * Species-specific: massive size, high damage, large territory, slow but devastating.
 *
 * Stats (realistic approximation):
 *   Mass: ~8000 kg | Length: ~12m | Speed: ~20 km/h (550 UU/s)
 *   Bite force: ~57,000 N (highest of any land predator)
 *   Vision: binocular forward-facing, motion-sensitive
 *   Hearing: excellent low-frequency detection
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // --- Species-specific overrides ---

    /** Called when TRex detects prey — triggers charge sequence */
    UFUNCTION(BlueprintNativeEvent, Category = "TRex|Combat")
    void OnPreyDetected(AActor* PreyActor);
    virtual void OnPreyDetected_Implementation(AActor* PreyActor);

    /** Roar ability — stuns nearby prey, triggers fear response in other dinosaurs */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Charge attack — brief speed boost toward target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void InitiateCharge(AActor* Target);

    /** Returns true if TRex is currently in charge state */
    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsCharging() const { return bIsCharging; }

    // --- Overridden base behavior ---
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Roar radius — all prey within this range receive fear debuff */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities",
        meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float RoarRadius;

    /** Roar fear duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities",
        meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float RoarFearDuration;

    /** Charge speed multiplier (applied to base MaxWalkSpeed) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
        meta = (ClampMin = "1.0", ClampMax = "3.0"))
    float ChargeSpeedMultiplier;

    /** Charge duration in seconds before returning to normal speed */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
        meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float ChargeDuration;

    /** Cooldown between roars in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities",
        meta = (ClampMin = "5.0", ClampMax = "120.0"))
    float RoarCooldown;

    /** Current charge target */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State",
        meta = (AllowPrivateAccess = "true"))
    TWeakObjectPtr<AActor> ChargeTarget;

private:
    bool bIsCharging;
    bool bRoarOnCooldown;
    float ChargeElapsed;
    float BaseWalkSpeed;

    FTimerHandle ChargeTimerHandle;
    FTimerHandle RoarCooldownHandle;

    void EndCharge();
    void ResetRoarCooldown();
};
