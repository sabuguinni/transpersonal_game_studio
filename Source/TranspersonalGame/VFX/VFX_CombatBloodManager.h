#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "VFXTypes.h"
#include "VFX_CombatBloodManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBloodEffectTriggered, FVector, ImpactLocation, float, Intensity);

/**
 * Manages realistic blood effects for dinosaur combat
 * Handles blood splatter, dripping, and pooling effects
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CombatBloodManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CombatBloodManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blood effect triggering
    UFUNCTION(BlueprintCallable, Category = "VFX|Blood")
    void TriggerBloodSplatter(const FVector& ImpactLocation, const FVector& ImpactNormal, float Damage);

    UFUNCTION(BlueprintCallable, Category = "VFX|Blood")
    void TriggerBloodDrip(const FVector& StartLocation, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Blood")
    void CreateBloodPool(const FVector& GroundLocation, float PoolSize = 100.0f);

    // Blood intensity management
    UFUNCTION(BlueprintCallable, Category = "VFX|Blood")
    void SetBloodIntensity(float NewIntensity);

    UFUNCTION(BlueprintPure, Category = "VFX|Blood")
    float GetCurrentBloodIntensity() const { return CurrentBloodIntensity; }

    // Cleanup
    UFUNCTION(BlueprintCallable, Category = "VFX|Blood")
    void CleanupBloodEffects();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "VFX|Events")
    FOnBloodEffectTriggered OnBloodEffectTriggered;

protected:
    // Niagara systems for blood effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood Systems")
    TSoftObjectPtr<UNiagaraSystem> BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood Systems")
    TSoftObjectPtr<UNiagaraSystem> BloodDripSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Blood Systems")
    TSoftObjectPtr<UNiagaraSystem> BloodPoolSystem;

    // Blood intensity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentBloodIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float MaxBloodEffectDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    int32 MaxActiveBloodEffects;

    // Runtime blood effect tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX|Runtime")
    TArray<UNiagaraComponent*> ActiveBloodEffects;

    UPROPERTY(BlueprintReadOnly, Category = "VFX|Runtime")
    float LastBloodEffectTime;

private:
    // Internal methods
    void SpawnBloodEffect(UNiagaraSystem* System, const FVector& Location, const FVector& Normal, float Scale = 1.0f);
    void CleanupOldBloodEffects();
    bool ShouldSpawnBloodEffect(const FVector& Location) const;
    float CalculateBloodIntensityFromDamage(float Damage) const;
};