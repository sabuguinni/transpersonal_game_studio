#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "SharedTypes.h"
#include "VFX_ImpactManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_ImpactType : uint8
{
    FootstepLight UMETA(DisplayName = "Light Footstep"),
    FootstepHeavy UMETA(DisplayName = "Heavy Footstep"),
    BloodSplatter UMETA(DisplayName = "Blood Splatter"),
    RockImpact UMETA(DisplayName = "Rock Impact"),
    WoodImpact UMETA(DisplayName = "Wood Impact"),
    WaterSplash UMETA(DisplayName = "Water Splash"),
    DustCloud UMETA(DisplayName = "Dust Cloud"),
    SparkShower UMETA(DisplayName = "Spark Shower")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ImpactData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* ParticleEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* ImpactSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector EffectScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToActor;

    FVFX_ImpactData()
    {
        ParticleEffect = nullptr;
        ImpactSound = nullptr;
        EffectScale = FVector(1.0f, 1.0f, 1.0f);
        Duration = 2.0f;
        bAttachToActor = false;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_ImpactManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_ImpactManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Impact System")
    TMap<EVFX_ImpactType, FVFX_ImpactData> ImpactEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    TArray<class UNiagaraComponent*> ActiveEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<class UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxConcurrentEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableDistanceCulling;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerImpact(EVFX_ImpactType ImpactType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, AActor* AttachActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerFootstepImpact(FVector Location, bool bIsHeavy = false, float IntensityMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerBloodEffect(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void TriggerDustCloud(FVector Location, float Radius = 100.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Impact")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintPure, Category = "VFX Impact")
    int32 GetActiveEffectCount() const;

protected:
    UFUNCTION()
    void CleanupFinishedEffects();

    UFUNCTION()
    void InitializeDefaultEffects();

    UFUNCTION()
    bool ShouldCullEffect(FVector EffectLocation) const;

private:
    float CurrentIntensityMultiplier;
    FTimerHandle CleanupTimerHandle;
};