// RaptorCharacter.h
// Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260624_001
// Velociraptor pack-hunter subclass of ADinosaurBase

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorCharacter.generated.h"

/**
 * ARaptorCharacter
 * Pack-hunter dinosaur. Lower health than T-Rex, higher speed.
 * Emits a pack call that alerts nearby raptors to converge on the target.
 * Designed for Agent #12 (Combat AI) to wire into Behavior Tree blackboard.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs",
       meta = (DisplayName = "Raptor Character"))
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

protected:
    virtual void BeginPlay() override;

public:
    // ── Pack System ──────────────────────────────────────────────────────────

    /** Radius within which other raptors hear the pack call (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack",
              meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float PackCallRadius;

    /** Cooldown between pack calls (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack",
              meta = (ClampMin = "1.0", ClampMax = "60.0"))
    float PackCallCooldown;

    /** Time of last pack call — used to enforce cooldown. */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    float LastPackCallTime;

    /** Maximum number of raptors that respond to a single pack call. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack",
              meta = (ClampMin = "1", ClampMax = "8"))
    int32 MaxPackResponders;

    // ── Pounce Attack ────────────────────────────────────────────────────────

    /** Pounce launch speed (cm/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat",
              meta = (ClampMin = "100.0", ClampMax = "3000.0"))
    float PounceSpeed;

    /** Damage dealt by a successful pounce. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat",
              meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float PounceDamage;

    /** Maximum horizontal distance from which a pounce can be initiated (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat",
              meta = (ClampMin = "50.0", ClampMax = "800.0"))
    float PounceRange;

    // ── Actions ──────────────────────────────────────────────────────────────

    /**
     * Emit a pack call: alerts all ARaptorCharacter actors within PackCallRadius
     * to set their attack target to this raptor's current target.
     * Respects PackCallCooldown.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Raptor|Pack")
    void PerformPackCall();
    virtual void PerformPackCall_Implementation();

    /**
     * Launch a pounce attack toward the target location.
     * Applies PounceDamage on hit via line-trace at apex.
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Raptor|Combat")
    void PerformPounce(AActor* Target);
    virtual void PerformPounce_Implementation(AActor* Target);

    /**
     * Claw slash — short-range melee, faster than bite.
     * Damage = 0.6 * AttackDamage (from DinosaurBase).
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Raptor|Combat")
    void PerformClawSlash();
    virtual void PerformClawSlash_Implementation();

    // ── Utility ──────────────────────────────────────────────────────────────

    /** Returns true if the pack call cooldown has elapsed. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Raptor|Pack")
    bool CanPerformPackCall() const;

    /** Returns all ARaptorCharacter actors within PackCallRadius of this raptor. */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    TArray<ARaptorCharacter*> GetNearbyPackMembers() const;
};
