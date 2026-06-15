#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "VFX_CampfireSystem.generated.h"

UENUM(BlueprintType)
enum class EVFX_CampfireState : uint8
{
    Unlit       UMETA(DisplayName = "Unlit"),
    Igniting    UMETA(DisplayName = "Igniting"),
    Burning     UMETA(DisplayName = "Burning"),
    Dying       UMETA(DisplayName = "Dying"),
    Extinguished UMETA(DisplayName = "Extinguished")
};

USTRUCT(BlueprintType)
struct FVFX_CampfireEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* FlameEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* SmokeEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* EmberEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UNiagaraSystem* SparkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* CracklingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float FlameIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float SmokeIntensity;

    FVFX_CampfireEffects()
    {
        FlameEffect = nullptr;
        SmokeEffect = nullptr;
        EmberEffect = nullptr;
        SparkEffect = nullptr;
        CracklingSound = nullptr;
        FlameIntensity = 1.0f;
        SmokeIntensity = 1.0f;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CampfireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CampfireSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* FlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* EmberComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SparkComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_CampfireEffects CampfireEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_CampfireState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float FuelLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float FuelConsumptionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float WindInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAffectedByWeather;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void IgniteCampfire();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void ExtinguishCampfire();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetWindStrength(float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetWeatherInfluence(bool bRaining, float RainIntensity);

    UFUNCTION(BlueprintPure, Category = "VFX State")
    EVFX_CampfireState GetCampfireState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "VFX State")
    float GetFuelLevel() const { return FuelLevel; }

    UFUNCTION(BlueprintPure, Category = "VFX State")
    bool IsBurning() const { return CurrentState == EVFX_CampfireState::Burning; }

private:
    void UpdateEffectIntensities();
    void UpdateAudioVolume();
    void TransitionToState(EVFX_CampfireState NewState);
    void CreateNiagaraComponents();

    float StateTransitionTimer;
    float LastSparkTime;
    float SparkInterval;
};