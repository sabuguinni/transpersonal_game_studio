#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/TimerHandle.h"
#include "VFX_CampfireEffectManager.generated.h"

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
struct FVFX_CampfireEffectData
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float IntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float VolumeMultiplier;

    FVFX_CampfireEffectData()
    {
        FlameEffect = nullptr;
        SmokeEffect = nullptr;
        EmberEffect = nullptr;
        SparkEffect = nullptr;
        CracklingSound = nullptr;
        IntensityMultiplier = 1.0f;
        VolumeMultiplier = 1.0f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCampfireStateChanged, EVFX_CampfireState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCampfireFuelChanged, float, FuelPercentage);

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CampfireEffectManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CampfireEffectManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Campfire Control
    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void IgniteCampfire();

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void ExtinguishCampfire();

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SetWindStrength(float WindStrength);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SetRainIntensity(float RainIntensity);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Campfire VFX")
    EVFX_CampfireState GetCampfireState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Campfire VFX")
    float GetFuelLevel() const { return CurrentFuel; }

    UFUNCTION(BlueprintPure, Category = "Campfire VFX")
    float GetIntensity() const { return CurrentIntensity; }

    // Effect Management
    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void UpdateEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Campfire VFX")
    void SpawnSparkBurst();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Campfire Events")
    FOnCampfireStateChanged OnStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Campfire Events")
    FOnCampfireFuelChanged OnFuelChanged;

protected:
    // VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* FlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* EmberComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UNiagaraComponent* SparkComponent;

    // Audio Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* AudioComponent;

    // Effect Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    TMap<EVFX_CampfireState, FVFX_CampfireEffectData> StateEffects;

    // Campfire Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float MaxFuel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float FuelConsumptionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire Settings", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float IgnitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire Settings", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float ExtinguishTime;

    // Environmental Factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Settings", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WindMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RainExtinguishThreshold;

private:
    // Internal State
    EVFX_CampfireState CurrentState;
    float CurrentFuel;
    float CurrentIntensity;
    float CurrentWindStrength;
    float CurrentRainIntensity;
    
    // Timers
    FTimerHandle StateTransitionTimer;
    FTimerHandle FuelConsumptionTimer;
    FTimerHandle SparkTimer;

    // Internal Methods
    void TransitionToState(EVFX_CampfireState NewState);
    void UpdateVFXForState();
    void UpdateAudioForState();
    void ConsumeFuel();
    void HandleIgnitionProcess();
    void HandleExtinguishProcess();
    void CalculateIntensity();
    void SpawnRandomSparks();
    void CheckEnvironmentalEffects();

    // Effect Spawning
    void SpawnFlameEffect();
    void SpawnSmokeEffect();
    void SpawnEmberEffect();
    void DestroyAllEffects();
};