// RaptorCharacter.h
// Performance Optimizer #04 | PROD_CYCLE_AUTO_20260622_002
// Velociraptor species — pack hunter, fast, low HP, high attack rate
// Inherits ADinosaurBase for full behaviour state machine

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "RaptorCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

protected:
    virtual void BeginPlay() override;

public:
    // ── Pack Hunting ──────────────────────────────────────────────────────────

    /** Radius to detect pack members (same species) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackDetectionRadius = 1200.0f;

    /** Damage bonus per additional pack member nearby (additive) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackDamageBonus = 8.0f;

    /** Max pack members that contribute to damage bonus */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackBonusMembers = 4;

    /** Current pack size (updated each tick via overlap) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    int32 CurrentPackSize = 0;

    /** True when flanking manoeuvre is active */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsFlankingTarget = false;

    // ── Combat ────────────────────────────────────────────────────────────────

    /** Base bite damage (low — compensated by attack rate and pack bonus) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float BaseBiteDamage = 45.0f;

    /** Claw slash damage — secondary attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float ClawDamage = 28.0f;

    /** Attack rate: attacks per second */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float AttackRate = 1.8f;

    /** Leap attack range — raptor jumps onto target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackRange = 400.0f;

    /** Leap attack cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown = 6.0f;

    /** True while leap attack is in progress */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Combat")
    bool bIsLeaping = false;

    // ── Movement ──────────────────────────────────────────────────────────────

    /** Sprint speed — raptors are fast (UU/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    float SprintSpeed = 1100.0f;

    /** Walk speed — stalking prey */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    float WalkSpeed = 420.0f;

    /** Jump Z velocity — raptors can jump over obstacles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Movement")
    float JumpVelocity = 550.0f;

    // ── Biology ───────────────────────────────────────────────────────────────

    /** Body mass in kg (Velociraptor mongoliensis ~15kg) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Biology")
    float BodyMassKg = 15.0f;

    /** Max HP — fragile but agile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Biology")
    float MaxHP = 180.0f;

    /** Territory radius (smaller than T-Rex — pack shares territory) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Biology")
    float TerritoryRadius = 3500.0f;

    // ── Actions ───────────────────────────────────────────────────────────────

    /** Perform bite attack — fast, low damage, high rate */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformBite();

    /** Perform claw slash — close range secondary */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformClawSlash();

    /** Leap onto target — closes distance, pins target briefly */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack(AActor* Target);

    /** Emit pack call — alerts nearby raptors to converge on target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void EmitPackCall(AActor* Target);

    /** Update pack size by scanning nearby raptors */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void UpdatePackSize();

    /** Calculate total damage including pack bonus */
    UFUNCTION(BlueprintPure, Category = "Raptor|Combat")
    float GetTotalBiteDamage() const;

protected:
    /** Apply raptor-specific stats to CharacterMovementComponent */
    void ApplyRaptorStats();

    /** Timer handle for leap cooldown */
    FTimerHandle LeapCooldownTimer;

    /** Timer handle for pack size update (runs every 2s) */
    FTimerHandle PackUpdateTimer;
};
