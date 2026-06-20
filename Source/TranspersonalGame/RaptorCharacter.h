// RaptorCharacter.h
// Core Systems Programmer #03 — Cycle PROD_CYCLE_AUTO_20260620_007
// Velociraptor subclass of ADinosaurBase — pack hunter, high speed, ambush predator

#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "RaptorCharacter.generated.h"

/**
 * ARaptorCharacter
 *
 * Velociraptor — pack hunter. High mobility, low individual health.
 * Coordinates with nearby pack members via PackLeader reference.
 * Executes flanking maneuvers and ambush attacks from cover.
 *
 * Species stats:
 *   MaxHealth     = 300
 *   MoveSpeed     = 700 (sprint), 450 (walk)
 *   AttackDamage  = 65
 *   bIsPack       = true
 *   PackSize      = 3-6
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Raptor Character"))
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

    // --- Pack System ---

    /** Reference to the pack leader (nullptr if this IS the leader) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    ARaptorCharacter* PackLeader;

    /** All pack members this raptor is aware of (populated by AI controller) */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<ARaptorCharacter*> PackMembers;

    /** Maximum number of raptors in this pack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize;

    /** True if this raptor is currently the pack leader */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsPackLeader;

    /** Radius within which pack members coordinate attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius;

    // --- Ambush System ---

    /** True when raptor is crouching in ambush position */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Ambush")
    bool bIsInAmbush;

    /** Minimum distance to target before breaking ambush and charging */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Ambush")
    float AmbushBreakDistance;

    /** Speed multiplier applied during the initial charge burst from ambush */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Ambush")
    float AmbushChargeMult;

    // --- Jump Attack ---

    /** True if raptor can currently perform a leap attack */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Combat")
    bool bCanLeapAttack;

    /** Cooldown in seconds between leap attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapAttackCooldown;

    /** Damage multiplier applied to leap attack (stacks with base AttackDamage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapDamageMult;

    /** Horizontal impulse applied to target on successful leap */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapKnockbackForce;

    // --- Vocalisation ---

    /** Sound cue played when calling pack members */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Audio")
    USoundBase* PackCallSound;

    /** Sound cue played on leap attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Audio")
    USoundBase* LeapAttackSound;

    // --- Interface ---

    /** Enter ambush crouch state */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Ambush")
    void EnterAmbush();

    /** Break ambush and charge target at full speed */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Ambush")
    void BreakAmbushAndCharge(AActor* Target);

    /** Perform leap attack toward target location */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void LeapAttack(AActor* Target);

    /** Broadcast pack call — alerts all pack members within PackCoordinationRadius */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void CallPack(AActor* ThreatTarget);

    /** Elect a new pack leader from surviving PackMembers */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ElectNewPackLeader();

    /** Returns true if pack has enough members to execute a coordinated flank */
    UFUNCTION(BlueprintPure, Category = "Raptor|Pack")
    bool CanFlank() const;

    // --- Overrides ---
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override death to trigger pack leadership election */
    virtual void OnDeath_Implementation() override;

protected:
    /** Internal timer handle for leap attack cooldown */
    FTimerHandle LeapCooldownTimer;

    /** Reset leap attack availability after cooldown */
    void ResetLeapAttack();
};
