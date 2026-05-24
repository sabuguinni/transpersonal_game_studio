#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_CombatEffects.generated.h"

UENUM(BlueprintType)
enum class EVFX_CombatType : uint8
{
    BloodSplatter,
    ImpactSpark,
    DustCloud,
    WeaponTrail,
    HitFlash
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_CombatSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    EVFX_CombatType EffectType = EVFX_CombatType::BloodSplatter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    FLinearColor TintColor = FLinearColor::Red;

    FVFX_CombatSettings()
    {
        EffectType = EVFX_CombatType::BloodSplatter;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        TintColor = FLinearColor::Red;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CombatEffects : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CombatEffects();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat VFX settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    FVFX_CombatSettings CombatSettings;

    // Niagara systems for different combat effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> BloodSplatterSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> ImpactSparkSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TSoftObjectPtr<UNiagaraSystem> WeaponTrailSystem;

    // Active Niagara components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat VFX")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Combat VFX functions
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerBloodSplatter(const FVector& Location, const FVector& Normal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerImpactSpark(const FVector& Location, const FVector& Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerDustCloud(const FVector& Location, float Radius = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void TriggerWeaponTrail(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void SetEffectScale(const FVector& NewScale);

private:
    void SpawnNiagaraEffect(UNiagaraSystem* System, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    void CleanupFinishedEffects();
};