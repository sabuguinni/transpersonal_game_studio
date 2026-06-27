#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "Velociraptor.generated.h"

/**
 * AVelociraptor — Fast pack-hunting carnivore.
 * Pack behavior: all raptors in radius share the same CurrentTarget via GameState lookup.
 * Stats: MaxHealth=80, AttackDamage=30, MoveSpeed=600, bIsPackHunter=true
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptor : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptor();

    // Pack behavior radius — raptors within this range share targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius;

    // Minimum pack size before initiating a coordinated attack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MinPackSizeForCoordination;

    // Jump attack: leap distance when pouncing on target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float PounceDistance;

    // Pounce damage multiplier (applied on top of base AttackDamage)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float PounceDamageMultiplier;

    // Whether this raptor is currently the pack leader (coordinates others)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    bool bIsPackLeader;

    // Number of pack members currently coordinating with this raptor
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    int32 ActivePackSize;

    // Flanking offset angle (degrees) — raptors spread around target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float FlankingAngleOffset;

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Share current target with nearby pack members
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void BroadcastTargetToPackMembers(AActor* NewTarget);

    // Receive a shared target from pack leader
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ReceivePackTarget(AActor* SharedTarget, AVelociraptor* Leader);

    // Perform pounce attack — leap at target dealing bonus damage
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformPounce();

    // Elect pack leader among nearby raptors (highest health = leader)
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void ElectPackLeader();

    // Get all pack members within PackCoordinationRadius
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    TArray<AVelociraptor*> GetNearbyPackMembers() const;

    // Override base attack to include pounce logic
    virtual void PerformAttack() override;

    // Override death to reassign pack leader
    virtual void OnDeath() override;

private:
    // Weak reference to current pack leader (null if this IS the leader)
    UPROPERTY()
    AVelociraptor* PackLeader;

    // Timer handle for pack coordination tick
    FTimerHandle PackCoordinationTimer;

    // Internal: run pack coordination logic
    void RunPackCoordination();

    // Internal: calculate flanking position around target
    FVector CalculateFlankingPosition(AActor* Target, float AngleOffset) const;
};
