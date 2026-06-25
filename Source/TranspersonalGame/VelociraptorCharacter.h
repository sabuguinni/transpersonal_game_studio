// VelociraptorCharacter.h — Velociraptor pack hunter subclass
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260625_007
#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Pack hunting radius — raptors within this range share attack bonuses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackRadius;

    // Damage multiplier when 2+ pack members are in range
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Pack")
    float PackBonusDamageMultiplier;

    // Leap attack range — raptor jumps onto target
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapRange;

    // Leap damage (base, before pack bonus)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapDamage;

    // Cooldown between leap attacks (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Raptor|Combat")
    float LeapCooldown;

    // How many pack members are currently in range (updated each tick)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Raptor|Pack")
    int32 NearbyPackCount;

    // Perform leap attack on target actor
    UFUNCTION(BlueprintCallable, Category = "Raptor|Combat")
    void PerformLeapAttack(AActor* Target);

    // Emit distress call — alerts nearby raptors to converge
    UFUNCTION(BlueprintCallable, Category = "Raptor|Pack")
    void EmitDistressCall();

    // Returns effective attack damage (base + pack bonus if applicable)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Raptor|Combat")
    float GetEffectiveAttackDamage() const;

private:
    FTimerHandle LeapCooldownTimer;
    bool bCanLeap;

    void ResetLeapCooldown();
    void UpdatePackCount();
};
