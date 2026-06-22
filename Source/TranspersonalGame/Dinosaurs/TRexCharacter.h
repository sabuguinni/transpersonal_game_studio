// TRexCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator species implementation.
// Inherits ADinosaurBase; overrides capsule, speed, attack, and territory values
// to match T-Rex biology: massive body, short sprint bursts, devastating bite.
//
// Compilation rules:
//   - All USTRUCT/UENUM at global scope (Rule 1)
//   - Unique type prefix "TRex_" (Rule 2)
//   - .generated.h LAST include (Rule 4)
//   - No escaped quotes (Rule 5)
//   - TRANSPERSONALGAME_API on exported class (Validation Rule 6)

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 *
 * Concrete Tyrannosaurus Rex implementation.
 * Stats tuned to real T-Rex biology:
 *   - Mass: ~8,000 kg → high knockback on attack
 *   - Sprint speed: ~28 km/h (780 UU/s) — fast for its size, not sustainable
 *   - Bite force: ~57,000 N → highest single-hit damage of all species
 *   - Territory radius: 5,000 UU (~50 m) — large exclusive zone
 *   - Hunger decay: slow (apex predator, large meals last longer)
 *
 * Blueprint subclassable for mesh/animation assignment without C++ changes.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "Dinosaurs",
       meta = (DisplayName = "T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // ─── Species Overrides ────────────────────────────────────────────────

    /** Maximum health pool — T-Rex is extremely resilient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats",
              meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float TRexMaxHealth = 2500.0f;

    /** Base walk speed (UU/s) — lumbering gait */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats",
              meta = (ClampMin = "50.0", ClampMax = "400.0"))
    float TRexWalkSpeed = 220.0f;

    /** Sprint speed (UU/s) — short burst charge */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats",
              meta = (ClampMin = "200.0", ClampMax = "900.0"))
    float TRexSprintSpeed = 780.0f;

    /** Bite damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats",
              meta = (ClampMin = "10.0", ClampMax = "1000.0"))
    float TRexBiteDamage = 350.0f;

    /** Attack range — long reach due to massive head */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats",
              meta = (ClampMin = "100.0", ClampMax = "600.0"))
    float TRexAttackRange = 280.0f;

    /** Territory radius — apex predators need large exclusive zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory",
              meta = (ClampMin = "1000.0", ClampMax = "20000.0"))
    float TRexTerritoryRadius = 8000.0f;

    /** Hunger decay rate per tick — slower than smaller species */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Survival",
              meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float TRexHungerDecayRate = 0.8f;

    // ─── Roar System ──────────────────────────────────────────────────────

    /** Whether the T-Rex is currently roaring (triggers animation + audio) */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Behaviour",
              meta = (AllowPrivateAccess = "true"))
    bool bIsRoaring = false;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour",
              meta = (ClampMin = "5.0", ClampMax = "120.0"))
    float RoarCooldown = 30.0f;

    /** Radius within which the roar intimidates prey (causes fear stat increase) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behaviour",
              meta = (ClampMin = "500.0", ClampMax = "5000.0"))
    float RoarIntimidationRadius = 2500.0f;

    /** Trigger a roar — intimidates nearby prey, announces territory */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void PerformRoar();

    /** Called when roar animation finishes */
    UFUNCTION(BlueprintCallable, Category = "TRex|Behaviour")
    void OnRoarFinished();

    // ─── Stomp System ─────────────────────────────────────────────────────

    /** Radius of ground stomp shockwave (knocks back small creatures) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
              meta = (ClampMin = "100.0", ClampMax = "1000.0"))
    float StompRadius = 400.0f;

    /** Damage dealt by stomp shockwave */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat",
              meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float StompDamage = 80.0f;

    /** Execute a ground stomp — radial damage + knockback */
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStomp();

    // ─── Overrides ────────────────────────────────────────────────────────

    virtual void BeginPlay() override;

    /** Override base attack to use TRexBiteDamage and TRexAttackRange */
    virtual void PerformAttack() override;

    /** Override die to play species-specific death sequence */
    virtual void Die() override;

protected:
    /** Apply T-Rex specific stats to base class properties */
    void ApplyTRexStats();

private:
    /** Timer handle for roar cooldown */
    FTimerHandle RoarCooldownTimer;

    /** Whether roar is currently on cooldown */
    bool bRoarOnCooldown = false;
};
