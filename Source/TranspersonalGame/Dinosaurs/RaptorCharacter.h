// RaptorCharacter.h
// Performance Optimizer #04 — PROD_CYCLE_AUTO_20260624_002
// Pack-hunter Velociraptor subclass of ADinosaurBase
// Species: Velociraptor | MaxHealth: 400 | AttackDamage: 60 | MaxWalkSpeed: 900

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

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Pack coordination ──────────────────────────────────────────────
    /** Pointer to the pack leader (nullptr if this IS the leader) */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    ARaptorCharacter* PackLeader;

    /** All pack members registered to this raptor (populated on leader only) */
    UPROPERTY(BlueprintReadWrite, Category = "Raptor|Pack")
    TArray<ARaptorCharacter*> PackMembers;

    /** True when this raptor is executing a flank manoeuvre */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsFlanking;

    /** Flank offset angle relative to pack leader's facing (degrees) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack", meta = (ClampMin = "-180.0", ClampMax = "180.0"))
    float FlankAngleDegrees;

    /** Radius within which pack members coordinate (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius;

    // ── Raptor abilities ───────────────────────────────────────────────
    /** Leap attack — lunge forward and pin target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack();

    /** Emit a pack-rally screech — alerts nearby raptors to converge */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void EmitPackScreech();

    /** Initiate flanking behaviour toward a target */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void BeginFlankManoeuvre(AActor* Target);

    /** Register a new pack member under this leader */
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void RegisterPackMember(ARaptorCharacter* NewMember);

    // ── State ──────────────────────────────────────────────────────────
    /** Current pack-hunt target (shared across pack) */
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|Pack")
    AActor* PackHuntTarget;

protected:
    /** Timer for periodic pack-coordination checks */
    FTimerHandle PackCoordTimerHandle;

    /** Timer for leap attack cooldown */
    FTimerHandle LeapCooldownHandle;

    /** Whether leap is on cooldown */
    bool bLeapOnCooldown;

    /** Periodic pack coordination tick */
    void CoordinatePack();

    /** Broadcast hunt target to all pack members */
    void BroadcastHuntTarget(AActor* Target);
};
