#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "Brachiosaurus.generated.h"

/**
 * ABrachiosaurus — Large herbivore sauropod.
 *
 * Behaviour rules:
 *  - Never hunts or attacks unless directly struck (bIsCarnivore = false)
 *  - Enters EWorld_DinoState::Fleeing when health drops below FleeHealthThreshold
 *  - Broadcasts a herd-flee signal to nearby Brachiosaurus within HerdAlertRadius
 *  - Performs a tail-swipe AoE when cornered (no escape route found)
 *  - Grazes periodically when in Idle state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABrachiosaurus : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ABrachiosaurus();

    // ── Herd behaviour ──────────────────────────────────────────────────────

    /** Radius within which this Brachiosaurus alerts herd-mates to flee */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Herd")
    float HerdAlertRadius = 3000.0f;

    /** Health fraction (0-1) below which this dinosaur flees */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Herd")
    float FleeHealthThreshold = 0.4f;

    /** Minimum distance to maintain from other herd members (loose spacing) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Herd")
    float HerdSpacingRadius = 600.0f;

    // ── Tail-swipe defence ──────────────────────────────────────────────────

    /** Damage dealt by the tail-swipe AoE when cornered */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float TailSwipeDamage = 120.0f;

    /** Radius of the tail-swipe sphere sweep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float TailSwipeRadius = 400.0f;

    /** Cooldown in seconds between tail swipes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Combat")
    float TailSwipeCooldown = 6.0f;

    // ── Grazing ─────────────────────────────────────────────────────────────

    /** How long (seconds) the dinosaur grazes in one idle session */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Grazing")
    float GrazeDuration = 8.0f;

    /** Interval between grazing sessions (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brachiosaurus|Grazing")
    float GrazeInterval = 20.0f;

    // ── Public interface ────────────────────────────────────────────────────

    /** Called by another herd member to trigger flee behaviour */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Herd")
    void ReceiveHerdFleeSignal(AActor* Threat);

    /** Perform tail-swipe AoE — damages all actors in TailSwipeRadius behind the dinosaur */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Combat")
    void PerformTailSwipe();

    /** Returns all nearby Brachiosaurus within HerdAlertRadius */
    UFUNCTION(BlueprintCallable, Category = "Brachiosaurus|Herd")
    TArray<ABrachiosaurus*> GetNearbyHerdMembers() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override: herbivore never hunts — only flees or defends */
    virtual void UpdateAIState() override;

    /** Override: on taking damage, broadcast flee to herd and check flee threshold */
    virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                              AController* EventInstigator, AActor* DamageCauser) override;

private:
    /** Broadcast flee signal to all herd members within HerdAlertRadius */
    void AlertHerd(AActor* Threat);

    /** Check if a valid escape path exists; if not, trigger tail swipe */
    void AttemptEscapeOrDefend(AActor* Threat);

    /** Start periodic grazing timer */
    void StartGrazingCycle();

    /** Called by grazing timer — toggles grazing state */
    void OnGrazeTimerTick();

    FTimerHandle GrazeTimerHandle;
    FTimerHandle TailSwipeCooldownHandle;

    bool bIsGrazing = false;
    bool bTailSwipeReady = true;
    float TimeSinceLastHerdCheck = 0.0f;
};
