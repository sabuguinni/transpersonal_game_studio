#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter — Tyrannosaurus Rex species implementation.
 *
 * Inherits from ADinosaurBase. Overrides stats with T-Rex specific values:
 * - Apex predator: high health, high damage, slow speed
 * - Aggressive carnivore: attacks on sight within 2500 units
 * - Territorial: does not flee unless health < 20%
 *
 * @author Core Systems Programmer — Agent #3
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    /** Override: T-Rex specific BeginPlay setup */
    virtual void BeginPlay() override;

    /** Roar ability — stuns nearby prey for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Stomp attack — AoE damage in StompRadius around feet */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Duration (seconds) prey is stunned after a roar */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunDuration = 3.0f;

    /** Radius (cm) of the stomp AoE damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 400.0f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 60.0f;

    /** Cooldown (seconds) between roars */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 15.0f;

    /** Whether the T-Rex is currently in a roar animation */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsRoaring = false;

protected:
    /** Internal timer tracking last roar time */
    float LastRoarTime = 0.0f;

    /** Apply T-Rex default stats in constructor */
    void InitTRexStats();
};
