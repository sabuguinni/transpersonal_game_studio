#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur — Tyrannosaurus Rex
 * Apex predator. Territorial, high damage, slow turn rate.
 * Hunts large prey, attacks player on sight within aggro radius.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "T-Rex Dinosaur"))
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- T-Rex Specific Properties ---

    /** Roar radius — triggers fear response in nearby creatures */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarRadius = 2000.0f;

    /** Roar cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 15.0f;

    /** Stomp damage radius — area damage on heavy footfall */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 300.0f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 80.0f;

    /** Whether T-Rex is currently in a roar animation */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsRoaring = false;

    /** Time since last roar */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastRoar = 0.0f;

    // --- T-Rex Specific Functions ---

    /** Trigger roar — applies fear to nearby creatures and player */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void PerformRoar();

    /** Perform stomp attack — area damage around feet */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Check if player is within vision cone */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsPlayerInVisionCone() const;

protected:
    /** Override base attack — T-Rex uses bite + stomp combo */
    virtual void PerformAttack() override;

    /** Override aggro check — T-Rex has wider detection but slower reaction */
    virtual bool ShouldAggro(AActor* Target) const override;

private:
    /** Internal timer for roar cooldown */
    float RoarTimer = 0.0f;
};
