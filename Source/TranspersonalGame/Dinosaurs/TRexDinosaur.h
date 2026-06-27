#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur — Tyrannosaurus Rex subclass.
 *
 * Apex predator. High health, devastating melee attack, poor turning radius.
 * Detects player by sight (wide forward cone) and sound (footsteps, sprinting).
 * Territorial: will patrol a 3000 UU radius around its spawn point.
 * Roars when entering Hunting state — triggers fear response on TranspersonalCharacter.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="T-Rex Dinosaur"))
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    // --- Species-specific overrides ---

    /** Roar ability: plays roar montage, applies fear to nearby player, cooldown 15s */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformRoar();

    /** Stomp attack: AoE ground slam dealing 80 damage in 400 UU radius */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformStomp();

    /** Called when TRex enters Hunting state — triggers roar */
    virtual void OnDetectPlayer(APawn* Player) override;

    /** TRex charges in a straight line when within 1500 UU of target */
    UFUNCTION(BlueprintCallable, Category="TRex|Movement")
    void StartChargeAttack(FVector TargetLocation);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- TRex-specific stats ---

    /** Roar fear radius — players within this range receive fear buildup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float RoarFearRadius = 2500.f;

    /** Fear amount applied per roar (0-100 scale matching SurvivalComponent) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float RoarFearAmount = 35.f;

    /** Stomp damage radius in Unreal Units */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float StompRadius = 400.f;

    /** Stomp damage dealt to anything in radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float StompDamage = 80.f;

    /** Charge speed multiplier applied to MaxSprintSpeed during charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Movement")
    float ChargeSpeedMultiplier = 1.4f;

    /** Charge duration in seconds before returning to normal movement */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Movement")
    float ChargeDuration = 2.5f;

    /** Cooldown between roars in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float RoarCooldown = 15.f;

    /** Cooldown between stomps in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Abilities")
    float StompCooldown = 8.f;

private:
    float LastRoarTime = -999.f;
    float LastStompTime = -999.f;
    bool bIsCharging = false;
    float ChargeEndTime = 0.f;
    FVector ChargeTargetLocation = FVector::ZeroVector;
};
