#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 *
 * Apex predator. High health, devastating attack damage, large detection radius.
 * Solitary hunter — does NOT use pack tactics.
 * Behaviour: Patrols territory, attacks anything in range, flees only at <10% health.
 *
 * Stats:
 *   Health:          500
 *   Attack Damage:    80
 *   Attack Range:    350 cm
 *   Detection:      1800 cm
 *   Move Speed:      350 cm/s
 *   Sprint Speed:    650 cm/s
 *   Mass:          8000 kg
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "TranspersonalGame|Dinosaurs")
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    virtual void BeginPlay() override;

    // ---- TRex-specific properties ----

    /** Roar cooldown in seconds — triggers fear response in nearby prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float RoarCooldown = 15.0f;

    /** Stomp radius — area-of-effect ground impact when charging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float StompRadius = 400.0f;

    /** Stomp damage applied to actors within StompRadius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour")
    float StompDamage = 40.0f;

    /** Whether the TRex is currently in a charge state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Behaviour")
    bool bIsCharging = false;

    // ---- TRex-specific functions ----

    /** Trigger a territorial roar — applies fear to nearby player/NPCs */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void TriggerRoar();

    /** Stomp attack — area damage around current position */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Override: on death, play collapse sequence */
    virtual void OnDinoDeath_Implementation() override;

    /** Override: on spotting target, trigger roar if cooldown elapsed */
    virtual void OnDinoSpotTarget_Implementation(AActor* SpottedTarget) override;

protected:
    float LastRoarTime = -999.0f;
};
