#pragma once

#include "CoreMinimal.h"
#include "AI/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter — Tyrannosaurus Rex apex predator subclass.
 *
 * Stats: MaxHealth=2000, AttackDamage=200, DetectionRadius=3000.
 * Behaviour: solitary ambush predator; charges when player enters detection radius;
 * roars before attacking (0.8s wind-up); cannot be stunned by small impacts.
 *
 * Inherits full state machine from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // ── Roar ability ──────────────────────────────────────────────────────────

    /** Play roar animation/sound and pause 0.8s before first attack. */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Whether TRex is currently in roar wind-up (blocks attack). */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Combat")
    bool bIsRoaring = false;

    // ── Stomp ability ─────────────────────────────────────────────────────────

    /** Stomp attack — deals 150% damage in 500-unit radius around feet. */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Stomp damage radius in cm. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 500.f;

    /** Stomp damage multiplier applied on top of base AttackDamage. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamageMultiplier = 1.5f;

    // ── Charge ability ────────────────────────────────────────────────────────

    /** Sprint charge — temporarily boosts MaxWalkSpeed to 1800 for 3 seconds. */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StartCharge();

    /** Duration of charge boost in seconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeDuration = 3.f;

    // ── Overrides ─────────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    /** Roar wind-up timer handle. */
    FTimerHandle RoarTimerHandle;

    /** Charge timer handle. */
    FTimerHandle ChargeTimerHandle;

    /** Called when roar wind-up completes — triggers first attack. */
    void OnRoarComplete();

    /** Called when charge duration expires — restores normal speed. */
    void OnChargeComplete();

    /** Normal sprint speed stored before charge boost. */
    float NormalSprintSpeed = 1200.f;
};
