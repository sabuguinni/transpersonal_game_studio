#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "VFX_CombatEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_CombatEffectType : uint8
{
    BloodImpact,
    BloodDrip,
    SpearPenetration,
    DinosaurBite,
    GroundSlam,
    RoarDistortion,
    PlayerExhaustion,
    WoundBleeding
};

USTRUCT(BlueprintType)
struct FVFX_CombatEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> EffectSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<USoundCue> EffectSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToActor = false;

    FVFX_CombatEffectData()
    {
        Duration = 2.0f;
        Intensity = 1.0f;
        bAttachToActor = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CombatEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CombatEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat VFX")
    TMap<EVFX_CombatEffectType, FVFX_CombatEffectData> CombatEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullDistance = 2000.0f;

    UPROPERTY()
    TArray<UNiagaraComponent*> ActiveEffects;

public:
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayCombatEffect(EVFX_CombatEffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayBloodImpact(FVector ImpactLocation, FVector ImpactNormal, float Damage = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlaySpearImpact(FVector Location, FVector Direction, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayDinosaurBiteEffect(FVector Location, AActor* Victim);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayGroundSlamEffect(FVector Location, float Radius = 300.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void PlayRoarDistortionEffect(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void StartPlayerExhaustionEffect(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void StopPlayerExhaustionEffect();

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void StartWoundBleedingEffect(AActor* Actor, FVector WoundLocation);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void StopWoundBleedingEffect(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantEffects(FVector ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupFinishedEffects();

private:
    void InitializeCombatEffects();
    UNiagaraComponent* SpawnEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation, AActor* AttachActor = nullptr);
    void ManageEffectPool();

    UPROPERTY()
    UNiagaraComponent* PlayerExhaustionComponent;

    UPROPERTY()
    TMap<AActor*, UNiagaraComponent*> ActiveWoundEffects;
};