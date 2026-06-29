#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator species — largest carnivore in the prehistoric world.
 * Inherits all base behavior from ADinosaurBase.
 * Stats: high health, high damage, slow speed, large territory, solo hunter.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

protected:
    virtual void BeginPlay() override;
    virtual void InitializeSpeciesDefaults() override;

public:
    /** Roar ability — stuns nearby prey for a short duration */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Stomp attack — AoE damage in a radius around the TRex */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStomp();

    /** Returns true if the TRex is in its charging animation phase */
    UFUNCTION(BlueprintPure, Category = "TRex|State")
    bool IsCharging() const { return bIsCharging; }

    /** Charge duration in seconds before the TRex reaches full sprint */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeWindupTime = 1.5f;

    /** Roar stun radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunRadius = 1500.0f;

    /** Roar stun duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunDuration = 3.0f;

    /** Stomp damage radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 500.0f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 75.0f;

    /** Cooldown between roar uses in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 15.0f;

    /** Cooldown between stomp uses in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompCooldown = 8.0f;

private:
    bool bIsCharging = false;
    float LastRoarTime = -999.0f;
    float LastStompTime = -999.0f;

    /** Apply stun to all actors within RoarStunRadius */
    void ApplyRoarStun();

    /** Apply stomp AoE damage to actors within StompRadius */
    void ApplyStompDamage();
};
