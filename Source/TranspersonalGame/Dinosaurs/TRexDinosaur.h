// TRexDinosaur.h
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Tyrannosaurus Rex — apex predator, aggressive, high HP, devastating attack

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRexDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs")
class TRANSPERSONALGAME_API ATRexDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexDinosaur();

    // TRex-specific: roar ability that frightens nearby prey
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformRoar();

    // TRex-specific: stomp attack — AoE damage in melee range
    UFUNCTION(BlueprintCallable, Category = "TRex|Abilities")
    void PerformStomp();

    // Called when TRex detects player — triggers aggressive chase
    UFUNCTION(BlueprintNativeEvent, Category = "TRex|AI")
    void OnPlayerDetected(AActor* PlayerActor);
    virtual void OnPlayerDetected_Implementation(AActor* PlayerActor);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Roar cooldown timer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float RoarCooldownRemaining;

    // Stomp cooldown timer
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State", meta = (AllowPrivateAccess = "true"))
    float StompCooldownRemaining;

    // Roar radius — all prey within this range gain FearLevel boost
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarRadius;

    // Roar fear amount added to prey
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarFearAmount;

    // Stomp radius — AoE damage radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompRadius;

    // Stomp damage dealt to all actors in radius
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompDamage;

    // Roar cooldown duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float RoarCooldownDuration;

    // Stomp cooldown duration in seconds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Config")
    float StompCooldownDuration;

    // Whether TRex is currently in a territorial patrol
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TRex|State")
    bool bIsPatrolling;

    // Territory center — TRex patrols within TerritoryRadius of this point
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    FVector TerritoryCenter;

    // Territory radius — TRex stays within this range of TerritoryCenter
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Territory")
    float TerritoryRadius;

private:
    FTimerHandle RoarCooldownTimer;
    FTimerHandle StompCooldownTimer;

    void ResetRoarCooldown();
    void ResetStompCooldown();
};
