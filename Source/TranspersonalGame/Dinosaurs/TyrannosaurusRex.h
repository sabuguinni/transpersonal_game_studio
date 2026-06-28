// TyrannosaurusRex.h — Tyrannosaurus Rex dinosaur class
// Agent #3 — Core Systems Programmer
// Inherits from ADinosaurBase. Apex predator: high health, high damage, aggressive.

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 *
 * Apex predator of the prehistoric world. Extremely high health and damage output.
 * Aggressive by default — will attack the player on sight within detection range.
 * Large capsule (120 radius, 220 half-height). Slow but devastating.
 *
 * Gameplay role: Boss-tier encounter. Player must avoid or use terrain to escape.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Tyrannosaurus Rex"))
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    // Override BeginPlay to set TRex-specific AI patrol radius
    virtual void BeginPlay() override;

    // Override Tick for roar cooldown logic
    virtual void Tick(float DeltaTime) override;

    // Roar ability — intimidates nearby creatures, triggers fear in player
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    // Stomp attack — area damage in front of TRex
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStompAttack();

    // Called when TRex detects player — triggers roar if cooldown expired
    virtual void OnTargetDetected_Implementation(AActor* Target) override;

    // Called when TRex kills a target — brief feeding behavior
    virtual void OnDeath_Implementation() override;

    // Time since last roar (seconds)
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastRoar;

    // Roar cooldown in seconds (default 45s)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarCooldown;

    // Stomp damage radius (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompRadius;

    // Stomp damage amount
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompDamage;

    // Whether TRex is currently in feeding state after kill
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsFeeding;

    // Feeding duration (seconds) before resuming patrol
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float FeedingDuration;

private:
    float FeedingTimer;
};
