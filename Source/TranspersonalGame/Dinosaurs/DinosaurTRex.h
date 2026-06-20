// DinosaurTRex.h
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_004
// T-Rex species: apex predator, massive health pool, devastating attack, limited turn rate

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "DinosaurTRex.generated.h"

/**
 * ADinosaurTRex
 * Tyrannosaurus Rex — apex predator of the prehistoric world.
 * Stats: MaxHealth=2000, AttackDamage=200, MoveSpeed=350, DetectionRadius=4000
 * Signature abilities: Roar stun (inherited from base), Stomp AoE, limited turn rate
 * Behavior: Solitary, territorial, attacks on sight within detection radius
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurTRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ADinosaurTRex();

    // --- Stomp AoE Attack ---
    /** Radius of the ground stomp AoE damage zone (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 400.0f;

    /** Damage dealt by stomp AoE to all actors in radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 120.0f;

    /** Cooldown between stomp attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompCooldown = 8.0f;

    /** Stomp knockback force applied to hit actors */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompKnockbackForce = 1200.0f;

    // --- Roar ---
    /** Radius within which roar stuns prey (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunRadius = 1500.0f;

    /** Duration of roar stun effect on prey (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarStunDuration = 2.5f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown = 20.0f;

    // --- Movement Constraints ---
    /** Maximum turn rate (degrees/second) — T-Rex turns slowly */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float MaxTurnRate = 45.0f;

    /** T-Rex cannot jump — enforced in constructor */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Movement")
    bool bCanJump = false;

    // --- Territorial Behavior ---
    /** Radius of T-Rex territory (cm). Intruders trigger aggression. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float TerritoryRadius = 8000.0f;

    /** Whether this T-Rex is currently defending territory */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|Behavior")
    bool bIsDefendingTerritory = false;

    /** Center of this T-Rex's territory (set at spawn) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    FVector TerritoryCenter = FVector::ZeroVector;

    // --- Internal State ---
    /** Time since last stomp (tracks cooldown) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastStomp = 0.0f;

    /** Time since last roar (tracks cooldown) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    float TimeSinceLastRoar = 0.0f;

    // --- Abilities ---
    /** Execute stomp AoE attack — damages all actors in StompRadius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    /** Execute roar — stuns all prey within RoarStunRadius */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformRoar();

    /** Check if stomp is off cooldown */
    UFUNCTION(BlueprintPure, Category = "TRex|Combat")
    bool CanStomp() const;

    /** Check if roar is off cooldown */
    UFUNCTION(BlueprintPure, Category = "TRex|Combat")
    bool CanRoar() const;

    /** Set territory center to current location (call at spawn) */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void ClaimTerritory();

    /** Returns true if location is within territory radius */
    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    bool IsLocationInTerritory(FVector Location) const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override death — T-Rex death triggers ground shake effect */
    virtual void OnDeath_Implementation() override;
};
