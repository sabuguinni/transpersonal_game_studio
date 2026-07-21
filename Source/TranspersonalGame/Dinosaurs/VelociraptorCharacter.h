#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

/**
 * AVelociraptorCharacter — Pack-hunting predator.
 * 
 * Velociraptor is a fast, coordinated pack hunter. Individually weak but
 * deadly in groups. Coordinates with nearby pack members to flank targets.
 * 
 * Key stats vs ADinosaurBase defaults:
 *   MaxHealth:    150  (fragile — dies in 3 hits from player)
 *   AttackDamage:  30  (low per-hit, but pack multiplies this)
 *   RunSpeed:    1200  (fastest dino — player cannot outrun)
 *   DetectionRadius: 2500u (keen senses, spots player early)
 *   bIsPackHunter: true (coordinates with other raptors)
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Pack Coordination ─────────────────────────────────────────────────

    /** Maximum distance to search for pack members to coordinate with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius = 1500.0f;

    /** How many pack members are currently in range */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    int32 NearbyPackMemberCount = 0;

    /** Damage multiplier applied per additional pack member in range (stacks up to 3) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackDamageMultiplier = 1.25f;

    /** Minimum pack size to trigger coordinated flank attack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 FlankThreshold = 2;

    // ── Raptor-specific Abilities ─────────────────────────────────────────

    /** Pounce — leap at target, dealing bonus damage on landing */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformPounce(AActor* Target);

    /** Screech — alerts nearby pack members and increases their aggression */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void PerformScreech();

    /** Returns effective damage accounting for pack bonus */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    float GetEffectiveAttackDamage() const;

    /** Returns true if this raptor is currently the pack leader */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    bool IsPackLeader() const;

    // ── Blueprint Events ──────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Raptor|Events")
    void OnPounce(AActor* Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Raptor|Events")
    void OnScreech();

    UFUNCTION(BlueprintImplementableEvent, Category = "Raptor|Events")
    void OnPackMemberJoined(AVelociraptorCharacter* NewMember);

protected:
    /** Pounce cooldown timer handle */
    FTimerHandle PounceTimerHandle;

    /** Screech cooldown timer handle */
    FTimerHandle ScreechTimerHandle;

    /** Whether pounce is currently on cooldown */
    bool bPounceCooldown = false;

    /** Whether screech is currently on cooldown */
    bool bScreechCooldown = false;

    /** Pounce cooldown duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float PounceCooldownDuration = 4.0f;

    /** Screech cooldown duration in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float ScreechCooldownDuration = 8.0f;

    /** Pounce bonus damage (added on top of base AttackDamage) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float PounceBonusDamage = 20.0f;

    /** Whether this raptor is the designated pack leader */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack", meta = (AllowPrivateAccess = "true"))
    bool bIsLeader = false;

    /** Pack coordination tick — runs every BehaviorTickInterval */
    void UpdatePackCoordination();

    /** Find and count nearby Velociraptor pack members */
    int32 CountNearbyPackMembers() const;

    /** Reset pounce cooldown */
    void ResetPounceCooldown();

    /** Reset screech cooldown */
    void ResetScreechCooldown();
};
