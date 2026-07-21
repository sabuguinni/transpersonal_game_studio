#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "VelociraptorDinosaur.generated.h"

/**
 * AVelociraptorDinosaur — Pack hunter, fast ambush predator
 * Speed: 900 cm/s, Pack coordination, Leap attack
 * Performance budget: lightweight AI, shared pack state pointer
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorDinosaur();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Pack Coordination ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    bool bIsPackLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 PackID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackCoordinationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    int32 MaxPackSize;

    // --- Leap Attack ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapImpulseForce;

    // --- Flanking ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Tactics")
    float FlankingAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Tactics")
    bool bIsFlankingTarget;

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Raptor|State")
    bool bIsLeaping;

    UPROPERTY(BlueprintReadOnly, Category = "Raptor|State")
    float LeapCooldownRemaining;

    // --- Abilities ---
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void ExecuteLeapAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void SignalPackToAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Raptor|Tactics")
    void ExecuteFlankingManeuver(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Raptor|State")
    bool CanLeap() const;

    UFUNCTION(BlueprintPure, Category = "Raptor|Pack")
    bool IsPackLeader() const { return bIsPackLeader; }

private:
    void UpdateLeapCooldown(float DeltaTime);
    void AttemptPackCoordination();
};
