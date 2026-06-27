#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TyrannosaurusRex.generated.h"

/**
 * ATyrannosaurusRex
 * Apex predator — large territory, high damage, slow turn rate, roar ability.
 * Inherits state machine from ADinosaurBase.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATyrannosaurusRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATyrannosaurusRex();

    /** Roar that frightens nearby prey — triggers flee state on smaller dinos */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Stomp attack — area damage in front of TRex */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void StompAttack();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Radius of roar fear effect (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarFearRadius = 2000.0f;

    /** Stomp damage radius (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompRadius = 400.0f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompDamage = 80.0f;

    /** Cooldown between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown = 15.0f;

    /** Time since last roar */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float TimeSinceLastRoar = 0.0f;

    /** Whether stomp is on cooldown */
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    bool bStompOnCooldown = false;

private:
    FTimerHandle StompCooldownTimer;

    void ResetStompCooldown();
};
