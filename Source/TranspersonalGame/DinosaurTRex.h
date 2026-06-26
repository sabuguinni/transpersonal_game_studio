// DinosaurTRex.h — Tyrannosaurus Rex subclass of ADinosaurBase
// Engine: Unreal Engine 5.5 | Module: TranspersonalGame
// Species: Tyrannosaurus Rex — apex predator, aggressive, high health/damage

#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "DinosaurTRex.generated.h"

/**
 * ATRexCharacter — Tyrannosaurus Rex
 *
 * Apex predator of the prehistoric world. Extremely aggressive, high health pool,
 * devastating attack damage. Territorial — will attack anything that enters its range.
 * Slow turn rate but high sprint speed. Roars when entering attack state.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dinosaur - T-Rex"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // T-Rex specific: territorial roar radius — anything entering triggers attack
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float TerritorialRadius = 1500.0f;

    // T-Rex specific: stomp attack — AoE damage in front
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 80.0f;

    // Cooldown between stomps (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompCooldown = 4.0f;

    // Roar cooldown (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 12.0f;

    // Execute stomp attack — applies AoE damage in front
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void ExecuteStomp();

    // Trigger territorial roar — alerts nearby dinos, frightens prey
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TriggerRoar();

    // Returns true if a target is within territorial radius
    UFUNCTION(BlueprintPure, Category = "TRex|Behavior")
    bool IsTargetInTerritorialRange(AActor* Target) const;

protected:
    virtual void OnDeath_Implementation() override;

private:
    float TimeSinceLastStomp = 0.0f;
    float TimeSinceLastRoar = 0.0f;
    bool bHasRoaredThisAttack = false;
};
