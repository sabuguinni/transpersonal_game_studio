#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur — Tyrannosaurus Rex species implementation.
 * Apex predator: high health, massive attack damage, wide detection radius,
 * slow turn rate, cannot jump. Hunts large prey, territorial vs other carnivores.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    /** Roar ability — stuns nearby prey and triggers flee in herbivores */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Stomp attack — AoE damage in front of TRex when target is very close */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStomp();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override attack to use stomp at close range */
    virtual void PerformAttack(AActor* Target) override;

    /** Roar cooldown in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarCooldown = 15.0f;

    /** Stomp AoE radius in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompRadius = 300.0f;

    /** Stomp AoE damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompDamage = 80.0f;

    /** Roar AoE radius — prey within this range will flee */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarRadius = 2000.0f;

    /** Blueprint event: roar VFX/SFX trigger */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnRoar();

    /** Blueprint event: stomp VFX/SFX trigger */
    UFUNCTION(BlueprintImplementableEvent, Category = "TRex|Events")
    void OnStomp();

private:
    float RoarTimer = 0.0f;
};
