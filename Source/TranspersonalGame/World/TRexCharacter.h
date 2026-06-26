#pragma once

#include "CoreMinimal.h"
#include "World/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter
 * Apex predator — Tyrannosaurus Rex.
 * Inherits ADinosaurBase and overrides stats for a large, powerful carnivore.
 * Designed for Blueprint subclassing: designers can tweak stats in BP_TRex
 * without touching C++.
 *
 * Key traits:
 *  - MaxHealth = 2000, AttackDamage = 200, DetectionRadius = 3000
 *  - Solitary hunter (no pack logic)
 *  - Stomps trigger camera shake on nearby players
 *  - Roar ability that temporarily suppresses nearby prey AI
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Roar ability — suppresses nearby prey AI for RoarSuppressDuration seconds */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    /** Stomp — deals radial damage in StompRadius and triggers camera shake */
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStomp();

    /** Time between roars (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarCooldown = 15.0f;

    /** Radius of stomp radial damage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompRadius = 400.0f;

    /** Stomp radial damage amount */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float StompDamage = 80.0f;

    /** Duration prey AI is suppressed after roar (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    float RoarSuppressDuration = 5.0f;

    /** Camera shake class triggered on stomp */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Abilities")
    TSubclassOf<UCameraShakeBase> StompCameraShakeClass;

protected:
    /** Cooldown timer for roar */
    float RoarCooldownRemaining = 0.0f;

    /** Whether currently suppressing nearby prey */
    bool bRoarActive = false;

    /** Remaining suppression time */
    float RoarSuppressRemaining = 0.0f;

    /** Override base stats in constructor */
    virtual void InitializeSpeciesStats();
};
