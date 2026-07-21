// TRexDinosaur.h
// Core Systems Programmer #03 — Tyrannosaurus Rex concrete class
// Inherits from ADinosaurBase — apex predator, solitary, ambush hunter

#pragma once

#include "CoreMinimal.h"
#include "World/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
    meta = (DisplayName = "Tyrannosaurus Rex"))
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── T-Rex specific properties ────────────────────────────────────────────

    /** Roar radius — triggers fear response in nearby prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float RoarRadius = 3000.f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float RoarCooldown = 30.f;

    /** Charge speed boost multiplier when closing on prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float ChargeSpeedMultiplier = 1.4f;

    /** Distance at which T-Rex initiates charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float ChargeInitiationDistance = 800.f;

    /** Whether T-Rex is currently in charge state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Behaviour")
    bool bIsCharging = false;

    // ── UFunctions ───────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void PerformRoar();

    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void StartCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void StopCharge();

    UFUNCTION(BlueprintCallable, Category = "TRex|Stats")
    float GetBiteForce() const;

protected:
    float TimeSinceLastRoar = 0.f;
    bool bChargeCooldown = false;
    float ChargeTimer = 0.f;
    static constexpr float MaxChargeDuration = 4.f;
};
