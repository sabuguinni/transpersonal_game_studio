// TyrannosaurusRex.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_005
// Tyrannosaurus Rex: apex predator, high health, devastating attack damage, moderate speed

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator of the prehistoric world. Slow but devastating.
 * MaxHealth=2000, AttackDamage=150, WalkSpeed=350, TerritoryRadius=3500
 * Detection range 2500u — will charge anything within territory.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    virtual void BeginPlay() override;

    /** Roar radius — nearby prey must pass a fear check or flee */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarRadius;

    /** Cooldown in seconds between roar events */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown;

    /** Stomp radius — ground shake effect when TRex moves */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompRadius;

    /** Triggers a roar — applies fear to nearby characters */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void Roar();

    /** Override: TRex charges when attacking (speed burst) */
    virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
        AController* EventInstigator, AActor* DamageCauser) override;

protected:
    /** Timer handle for periodic roar */
    FTimerHandle RoarTimerHandle;

    /** Triggers roar on a timer during Patrol/Attack states */
    void PeriodicRoar();
};
