#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "DinosaurTRex.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurTRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ADinosaurTRex();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // T-Rex specific stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float RoarRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float RoarCooldown;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Stats")
    float LastRoarTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float ChargeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "T-Rex Stats")
    float ChargeDistance;

    // T-Rex behavior states
    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    bool bIsRoaring;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    bool bIsCharging;

    UPROPERTY(BlueprintReadOnly, Category = "T-Rex Behavior")
    FVector ChargeTarget;

public:
    // T-Rex specific functions
    UFUNCTION(BlueprintCallable, Category = "T-Rex")
    void PerformRoar();

    UFUNCTION(BlueprintCallable, Category = "T-Rex")
    void StartCharge(FVector TargetLocation);

    UFUNCTION(BlueprintCallable, Category = "T-Rex")
    void StopCharge();

    UFUNCTION(BlueprintCallable, Category = "T-Rex")
    bool CanRoar();

    UFUNCTION(BlueprintCallable, Category = "T-Rex")
    void ScareNearbyDinosaurs();

protected:
    // Override base functions for T-Rex specific behavior
    virtual void AttackTarget() override;
    virtual void OnPawnSeen(APawn* SeenPawn) override;

private:
    void UpdateChargeMovement();
    void HandleRoarEffects();
};