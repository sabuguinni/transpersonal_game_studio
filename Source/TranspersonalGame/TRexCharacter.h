// TRexCharacter.h — Tyrannosaurus Rex subclass of ADinosaurBase
// Prehistoric Survival Game — Transpersonal Game Studio
// Agent #3 — Core Systems Programmer — PROD_CYCLE_AUTO_20260625_007
#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Apex predator. Slow, devastating, territorial.
 * MaxHealth=2000, AttackDamage=200, DetectionRadius=4000
 * Behaviour: solitary, charges on detection, roars before attacking.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    /** Roar radius — nearby prey flee when TRex roars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarRadius = 3000.f;

    /** Cooldown between roar events (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 12.f;

    /** Stomp damage radius — deals AoE damage when TRex stomps */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 300.f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 120.f;

    /** Whether TRex is currently in charge animation */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging = false;

    /** Trigger a roar — scares nearby prey, initiates attack state */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Stomp attack — AoE damage in StompRadius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Begin charge towards target location */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void BeginCharge(FVector TargetLocation);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float LastRoarTime = 0.f;

    /** Timer handle for charge duration */
    FTimerHandle ChargeTimerHandle;

    /** End charge after duration */
    void EndCharge();
};
