// TRexCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// T-Rex apex predator character — inherits ADinosaurBase
// Cycle: PROD_CYCLE_AUTO_20260630_010

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter — Tyrannosaurus Rex apex predator
 * 
 * Stats: MaxHealth=2000, AttackDamage=150, RunSpeed=800
 * Behaviour: Territorial, solitary, ambush predator
 * Special: Roar ability that causes Fear status on nearby players
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ─── T-Rex Specific Abilities ───────────────────────────────────────────

    /** Roar ability — causes Fear on nearby players within RoarRadius */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformRoar();

    /** Stomp attack — AoE damage in front of T-Rex */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformStomp();

    /** Bite attack — single target high damage */
    UFUNCTION(BlueprintCallable, Category="TRex|Abilities")
    void PerformBite();

    // ─── T-Rex Specific Properties ──────────────────────────────────────────

    /** Radius of roar fear effect (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float RoarRadius;

    /** Fear duration applied to players by roar (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float RoarFearDuration;

    /** Stomp AoE radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float StompRadius;

    /** Stomp AoE damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float StompDamage;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Stats")
    float RoarCooldown;

    /** Whether T-Rex is currently in ambush mode (reduced detection by prey) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TRex|State")
    bool bIsAmbushing;

    /** Territory radius — T-Rex will patrol and defend this area (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Territory")
    float TerritoryRadius;

    /** Home location for territory patrol */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TRex|Territory")
    FVector TerritoryCenter;

protected:
    /** Timer handle for roar cooldown */
    FTimerHandle RoarCooldownTimer;

    /** Whether roar is on cooldown */
    bool bRoarOnCooldown;

    /** Last time stomp was performed */
    float LastStompTime;

    /** Stomp cooldown (seconds) */
    float StompCooldown;

    /** Apply fear effect to a character */
    void ApplyFearToActor(AActor* Target, float Duration);

    /** Reset roar cooldown */
    void ResetRoarCooldown();

    /** Override base attack to use bite */
    virtual void PerformAttack() override;
};
