#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

/**
 * ATRexDinosaur — Tyrannosaurus Rex species implementation.
 *
 * Inherits from AEng_DinosaurBase and overrides stats with T-Rex specific values:
 * - High health (1200), high damage (150), slow speed (400 walk / 700 sprint)
 * - Large detection radius (3000 cm), long attack range (350 cm)
 * - Apex predator: hunts all prey, flees nothing
 * - Roar ability on aggro (audio cue + screen shake placeholder)
 *
 * Placed in MinPlayableMap as the primary apex threat.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "T-Rex Dinosaur"))
class TRANSPERSONALGAME_API ATRexDinosaur : public AEng_DinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    // Override — T-Rex roars when entering hunt state
    virtual void BeginPlay() override;

    // Override — stomp attack deals AoE damage in a radius
    virtual void PerformAttack() override;

    // Roar: plays sound + applies fear to nearby TranspersonalCharacters
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void Roar();

    // Stomp radius for AoE damage (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius;

    // Fear intensity applied to player on roar (0-1 range, fed into SurvivalComponent)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarFearIntensity;

    // Cooldown between roars (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float RoarCooldown;

protected:
    // Timer handle for roar cooldown
    FTimerHandle RoarCooldownHandle;

    // Whether roar is currently on cooldown
    bool bRoarOnCooldown;

    // Called when behavior state changes to Hunting — triggers roar
    void OnEnterHuntState();
};
