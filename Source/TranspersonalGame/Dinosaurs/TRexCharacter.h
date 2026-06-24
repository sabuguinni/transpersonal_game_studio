// TRexCharacter.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator subclass of ADinosaurBase

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Tyrannosaurus Rex — the apex predator of the prehistoric world.
 * Inherits all behavior state machine, combat, and survival logic from ADinosaurBase.
 * Species stats are configured in the constructor (health=2000, damage=150, speed=600).
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    /** Roar ability — intimidates nearby prey, triggers Fear stat on TranspersonalCharacter */
    UFUNCTION(BlueprintCallable, Category="TRex|Combat")
    void PerformRoar();

    /** Stomp attack — area damage in 300cm radius */
    UFUNCTION(BlueprintCallable, Category="TRex|Combat")
    void PerformStomp();

    /** Detection range override — TRex has excellent vision, 4000cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Perception")
    float RoarRadius = 1500.0f;

    /** Stomp damage radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float StompRadius = 300.0f;

    /** Stomp damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TRex|Combat")
    float StompDamage = 80.0f;

    /** Whether TRex is currently roaring */
    UPROPERTY(BlueprintReadOnly, Category="TRex|State")
    bool bIsRoaring = false;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    /** Timer handle for periodic roar */
    FTimerHandle RoarTimerHandle;

    /** Trigger roar on a schedule when hunting */
    void MaybeRoar();
};
