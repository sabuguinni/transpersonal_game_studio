#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "DinosaurTRex.generated.h"

/**
 * ATRex — Tyrannosaurus Rex concrete dinosaur class.
 * Inherits from ADinosaurBase (ACharacter subclass).
 * Species: EEng_DinoSpecies::TyrannosaurusRex
 * Role: Apex predator, solitary, extreme aggression, large detection radius.
 * Agent #12 (Combat AI) will attach a Behavior Tree to this class.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dinosaur T-Rex"))
class TRANSPERSONALGAME_API ATRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRex();

    /** Roar ability — stuns nearby prey for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Charge attack — sprint toward target and deal bonus damage */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ChargeAttack(AActor* Target);

    /** Duration prey is stunned after a roar (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunDuration = 3.0f;

    /** Multiplier applied to base bite damage during a charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDamageMultiplier = 2.5f;

    /** Radius of the roar stun effect (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarRadius = 1500.0f;

    /** Whether TRex is currently charging */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging = false;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    /** Cooldown timer for roar ability */
    float RoarCooldownRemaining = 0.0f;

    /** Cooldown between roars (seconds) */
    static constexpr float RoarCooldown = 15.0f;
};
