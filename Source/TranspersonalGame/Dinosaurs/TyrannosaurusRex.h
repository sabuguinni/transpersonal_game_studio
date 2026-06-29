#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 *
 * Apex predator dinosaur — the most dangerous creature in the prehistoric world.
 * Inherits from ADinosaurBase and overrides stats with T-Rex specific values:
 * - Health: 2000 HP (tank)
 * - Speed: 600 cm/s (fast for its size)
 * - Damage: 150 per bite
 * - Detection radius: 5000 cm (excellent vision)
 * - Territory radius: 8000 cm
 * - Diet: Carnivore (always hunts players on detection)
 *
 * Behavior: Solitary apex predator. Patrols territory, attacks anything that moves.
 * Roars when entering aggressive state. Charges at detected prey.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    /** T-Rex roar sound — played when entering aggressive state */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Audio")
    USoundBase* RoarSound;

    /** Stomp radius — ground shake effect when T-Rex walks nearby */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius;

    /** Stomp damage — applied to players within stomp radius on heavy footfall */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage;

    /** Charge speed multiplier — how much faster T-Rex moves when charging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeedMultiplier;

    /** Whether T-Rex is currently in a charge attack */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCharging;

    /** Charge cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeCooldown;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override: T-Rex roars when it detects a player */
    virtual void OnPlayerDetected(APawn* DetectedPlayer) override;

    /** Override: T-Rex death — collapses with heavy impact */
    virtual void Die() override;

    /** Initiate a charge attack toward target */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StartCharge(AActor* Target);

    /** Apply stomp damage to nearby players */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ApplyStompDamage();

private:
    float LastChargeTime;
    float LastStompTime;
    float StompInterval;
};
