#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator — highest health, massive damage, wide detection radius.
 * Solitary hunter, charges when target enters detection zone.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    virtual void BeginPlay() override;

    // ── T-Rex specific ────────────────────────────────────────────────────
    /** Roar radius — nearby prey panics (applies fear to player) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex")
    float RoarRadius = 3500.0f;

    /** Cooldown between roars in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex")
    float RoarCooldown = 15.0f;

    /** Charge speed multiplier when closing distance on prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex")
    float ChargeSpeedMultiplier = 1.6f;

    /** Whether T-Rex is currently in charge state */
    UPROPERTY(BlueprintReadOnly, Category = "TRex")
    bool bIsCharging = false;

    /** Trigger roar — Blueprint adds audio/VFX */
    UFUNCTION(BlueprintCallable, Category = "TRex")
    void PerformRoar();

    /** Initiate charge toward target */
    UFUNCTION(BlueprintCallable, Category = "TRex")
    void StartCharge(AActor* Target);

    /** Blueprint event: roar triggered */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnRoar();

    /** Blueprint event: charge initiated */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnChargeStart(AActor* Target);

protected:
    float RoarTimer = 0.0f;
};
