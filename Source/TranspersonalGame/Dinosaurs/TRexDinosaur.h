// TRexDinosaur.h — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260702_002
// Tyrannosaurus Rex — apex predator, slow but devastating
// Inherits from ADinosaurBase, overrides species-specific behavior

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="T-Rex Dinosaur"))
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // TRex-specific: charge attack when prey is in range
    UFUNCTION(BlueprintCallable, Category="TRex|Combat")
    void StartChargeAttack(AActor* Target);

    // TRex-specific: roar to alert nearby raptors (pack coordination)
    UFUNCTION(BlueprintCallable, Category="TRex|Behavior")
    void Roar();

    // TRex-specific: stomp shockwave — stagger nearby small creatures
    UFUNCTION(BlueprintCallable, Category="TRex|Combat")
    void StompAttack();

    // Is TRex currently charging?
    UFUNCTION(BlueprintPure, Category="TRex|State")
    bool IsCharging() const { return bIsCharging; }

    // Charge speed multiplier (applied during charge)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float ChargeSpeedMultiplier = 2.5f;

    // Charge attack damage (higher than normal bite)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float ChargeDamage = 150.0f;

    // Stomp radius — creatures within this radius get staggered
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float StompRadius = 400.0f;

    // Stomp stagger damage (non-lethal, causes stumble)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float StompStaggerDamage = 20.0f;

    // Roar radius — alerts raptors and other predators
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Behavior")
    float RoarRadius = 2000.0f;

    // Cooldown between charge attacks (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float ChargeCooldown = 8.0f;

    // Cooldown between roars
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Behavior")
    float RoarCooldown = 15.0f;

protected:
    // Override base behavior hooks
    virtual void OnIdle() override;
    virtual void OnRoam() override;
    virtual void OnAlert() override;
    virtual void OnAttack() override;
    virtual void OnFlee() override;
    virtual void OnDeath() override;

private:
    bool bIsCharging = false;
    float LastChargeTime = -999.0f;
    float LastRoarTime = -999.0f;
    AActor* ChargeTarget = nullptr;

    FTimerHandle ChargeTimer;
    FTimerHandle RoarTimer;

    void EndCharge();
    void PerformBiteAttack();
};
