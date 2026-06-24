#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

/**
 * AVelociraptorCharacter
 * Pack-hunting raptor subclass of ADinosaurBase.
 * 
 * Key behaviours:
 *  - Flanking AI: raptors in a pack spread out and attack from different angles
 *  - High speed (ChaseSpeed=800), low health (MaxHealth=200)
 *  - Pack coordination: one raptor distracts, others flank
 *  - Leap attack: closes distance instantly when in leap range
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

    // ─── Pack System ─────────────────────────────────────────────────────────

    /** Other raptors in this pack (max 3). Populated at spawn by the pack spawner. */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<AVelociraptorCharacter*> PackMembers;

    /** Index in the pack (0 = alpha/distractor, 1-2 = flankers). */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    int32 PackIndex;

    /** Angle offset for flanking (degrees). Alpha=0, Flanker1=90, Flanker2=-90. */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    float FlankAngleOffset;

    // ─── Leap Attack ─────────────────────────────────────────────────────────

    /** Distance at which the raptor performs a leap attack. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapRange;

    /** Impulse force applied during leap. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapImpulse;

    /** Cooldown between leap attacks (seconds). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown;

    /** Whether the raptor is currently in a leap. */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Combat")
    bool bIsLeaping;

    // ─── Overrides ───────────────────────────────────────────────────────────

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override: raptors use flanking movement instead of direct chase. */
    virtual void TickChase(float DeltaTime) override;

    /** Override: raptors leap when close enough. */
    virtual void TickAttack(float DeltaTime) override;

    /** Override: set raptor-specific stats and mesh. */
    virtual void OnDinoStateChanged_Implementation(EEng_DinoState NewState) override;

    /** Override: pack members scatter on death. */
    virtual void OnDinoDied_Implementation() override;

    // ─── Pack Coordination ───────────────────────────────────────────────────

    /** Register this raptor in a pack. Called by the pack spawner. */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void JoinPack(const TArray<AVelociraptorCharacter*>& Pack, int32 Index);

    /** Called when the alpha raptor spots the player — alerts all pack members. */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void AlertPack(AActor* Target);

    /** Compute the flanking destination for this raptor around the target. */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    FVector ComputeFlankPosition(AActor* Target) const;

    // ─── Leap ────────────────────────────────────────────────────────────────

    /** Attempt a leap attack toward the target. */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void AttemptLeap(AActor* Target);

private:
    float LeapCooldownTimer;

    /** Move toward the flank position each tick. */
    void UpdateFlankMovement(float DeltaTime);
};
