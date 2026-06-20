#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur
 * Apex predator — 500 HP, 600 move speed, 150 attack damage.
 * Carnivore. Highly aggressive when prey detected within DetectionRadius.
 * Roar ability that stuns nearby prey for 2 seconds.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /** Roar — stuns all actors within RoarRadius for RoarStunDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void Roar();

    /** Override base attack — T-Rex bite applies bleed damage over time */
    virtual void Attack_Implementation() override;

    /** Override death — T-Rex collapse sequence */
    virtual void OnDeath_Implementation() override;

    // ── Species Stats ──────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarStunDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float BleedDamagePerSecond = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float BleedDuration = 5.0f;

    /** Cooldown between roars in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Stats")
    float RoarCooldown = 15.0f;

private:
    bool bCanRoar = true;
    FTimerHandle RoarCooldownTimer;

    void ResetRoarCooldown();
};
