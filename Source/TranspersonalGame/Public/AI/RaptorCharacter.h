#pragma once

#include "CoreMinimal.h"
#include "AI/DinosaurBase.h"
#include "RaptorCharacter.generated.h"

/**
 * ARaptorCharacter — Velociraptor pack hunter
 * Extends ADinosaurBase with pack coordination, flanking, and leap attack.
 * Pack members share a target reference so they coordinate attacks.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Raptor Character"))
class TRANSPERSONALGAME_API ARaptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ARaptorCharacter();

    // ── Pack Coordination ─────────────────────────────────────────
    /** True for all raptors — enables pack AI logic in DinosaurAIController */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack")
    bool bIsPackHunter = true;

    /** Shared pack target — set by the alpha, read by all pack members */
    UPROPERTY(BlueprintReadWrite, Category = "Pack")
    AActor* PackTarget = nullptr;

    /** Max number of raptors that form a coordinated pack */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack")
    int32 MaxPackSize = 4;

    /** Radius within which pack members detect and share targets */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pack")
    float PackCoordinationRadius = 2500.0f;

    // ── Abilities ─────────────────────────────────────────────────
    /** Leap attack: lunges forward and pins target for 1.5s */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Abilities")
    void PerformLeapAttack();

    /** Flanking move: circle-strafe to target's side before attacking */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Abilities")
    void PerformFlankingManeuver();

    /** Broadcast target to all nearby pack members */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ShareTargetWithPack(AActor* NewTarget);

    /** Called when this raptor becomes the alpha (first to detect target) */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void BecomePackAlpha();

    // ── Overrides ─────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

protected:
    /** True if this raptor is the pack alpha (target broadcaster) */
    UPROPERTY(BlueprintReadOnly, Category = "Pack", meta = (AllowPrivateAccess = "true"))
    bool bIsAlpha = false;

    /** Cooldown timer for leap attack */
    FTimerHandle LeapCooldownTimer;

    /** Cooldown timer for flanking maneuver */
    FTimerHandle FlankCooldownTimer;

    /** Whether leap is currently on cooldown */
    bool bLeapOnCooldown = false;

    /** Whether flank is currently on cooldown */
    bool bFlankOnCooldown = false;

    /** Leap attack cooldown in seconds */
    UPROPERTY(EditAnywhere, Category = "Raptor|Abilities")
    float LeapCooldown = 6.0f;

    /** Flanking cooldown in seconds */
    UPROPERTY(EditAnywhere, Category = "Raptor|Abilities")
    float FlankCooldown = 4.0f;

    /** Leap impulse force applied to this raptor */
    UPROPERTY(EditAnywhere, Category = "Raptor|Abilities")
    float LeapImpulse = 1200.0f;

    /** Damage dealt on successful leap pin */
    UPROPERTY(EditAnywhere, Category = "Raptor|Abilities")
    float LeapDamage = 60.0f;

    /** Pin duration after leap lands */
    UPROPERTY(EditAnywhere, Category = "Raptor|Abilities")
    float PinDuration = 1.5f;

    void ResetLeapCooldown();
    void ResetFlankCooldown();
};
