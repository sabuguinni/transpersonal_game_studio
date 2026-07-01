// TRexCharacter.h
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Tyrannosaurus Rex — apex predator, 2000 HP, 150 damage per bite
// Inherits from ADinosaurBase

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexCharacter.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // TRex-specific attack: Bite with massive damage
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformBiteAttack();

    // Stomp attack — AoE damage in small radius
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void PerformStompAttack();

    // Territorial roar that scares smaller dinosaurs
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    void TerritorialRoar();

    // TRex charges toward target when health > 50%
    UFUNCTION(BlueprintCallable, Category = "TRex|Movement")
    void ChargeTowardTarget(AActor* Target);

    // Override: TRex is immune to fear (apex predator)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TRex|Behavior")
    bool IsFearless() const;
    virtual bool IsFearless_Implementation() const;

protected:
    // Bite cooldown timer
    FTimerHandle BiteAttackTimer;

    // Stomp cooldown timer
    FTimerHandle StompAttackTimer;

    // Charge state
    UPROPERTY(BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    bool bIsCharging;

    // Charge speed multiplier (3x normal speed)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Movement")
    float ChargeSpeedMultiplier;

    // Stomp radius (units)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius;

    // Stomp damage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage;

    // Bite cooldown (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float BiteCooldown;

    // Territorial radius — TRex defends this zone
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float TerritorialRadius;

    // Home territory center
    UPROPERTY(BlueprintReadOnly, Category = "TRex|Behavior", meta = (AllowPrivateAccess = "true"))
    FVector TerritoryCenter;

    // Check if intruder is in territory
    UFUNCTION(BlueprintCallable, Category = "TRex|Behavior")
    bool IsInTerritory(AActor* Actor) const;

    // Reset bite cooldown
    void ResetBiteCooldown();

    // Reset stomp cooldown
    void ResetStompCooldown();

    // Called when TRex enters charge state
    void StartCharge(AActor* Target);

    // Called when charge ends
    void EndCharge();
};
