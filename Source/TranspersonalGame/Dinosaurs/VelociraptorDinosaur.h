#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "VelociraptorDinosaur.generated.h"

/**
 * AVelociraptorDinosaur — Pack hunter, high speed, coordinated ambush attacks.
 * Stats: HP=800, DMG=120, Speed=1200, DetectionRadius=2500, Mass=80
 * Pack behaviour: bIsPackHunter=true, PackRadius=1500u, MaxPackSize=6
 * Abilities: LeapAttack(), FlankTarget(), CallPack()
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Dinosaurs))
class TRANSPERSONALGAME_API AVelociraptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorDinosaur();

    // --- Pack Hunt Abilities ---

    /** Leap at target — short-range burst (600u), deals 180 DMG on landing */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Abilities")
    void PerformLeapAttack();

    /** Flank target — circle strafe to attack from the side */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Abilities")
    void FlankTarget();

    /** Emit pack call — alerts all raptors within PackCallRadius to converge on target */
    UFUNCTION(BlueprintCallable, Category = "Dinosaur|Abilities")
    void CallPack();

    // --- Pack Configuration ---

    /** Maximum number of raptors that will join this pack */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    int32 MaxPackSize;

    /** Radius within which pack members coordinate attacks (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    float PackCoordinationRadius;

    /** Radius of pack call broadcast (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    float PackCallRadius;

    /** Leap attack damage (separate from base AttackDamage for tuning) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapDamage;

    /** Leap attack range (units) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapRange;

    /** Cooldown between leap attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float LeapCooldown;

    /** Cooldown between pack calls (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Combat")
    float PackCallCooldown;

    /** Is this raptor the pack alpha (leads flanking decisions)? */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur|Pack")
    bool bIsPackAlpha;

protected:
    virtual void BeginPlay() override;
    virtual void UpdateBehavior() override;

private:
    float LastLeapTime;
    float LastPackCallTime;

    /** Tracks current flank angle around target */
    float CurrentFlankAngle;

    /** Perform the actual leap movement and damage application */
    void ExecuteLeap(AActor* Target);

    /** Find all friendly raptors within PackCallRadius */
    TArray<AVelociraptorDinosaur*> FindPackMembers() const;
};
