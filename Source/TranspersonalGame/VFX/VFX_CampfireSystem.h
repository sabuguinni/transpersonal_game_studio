#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
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
struct TRANSPERSONALGAME_API FVFX_CampfireEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    class UNiagaraSystem* FlameSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    class UNiagaraSystem* SmokeSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    class UNiagaraSystem* EmberSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Effects")
    class UNiagaraSystem* SparkSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* CracklingSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float FireLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor FireLightColor;

    FVFX_CampfireEffects()
    {
        FlameSystem = nullptr;
        SmokeSystem = nullptr;
        EmberSystem = nullptr;
        SparkSystem = nullptr;
        CracklingSoundCue = nullptr;
        FireLightIntensity = 2000.0f;
        FireLightColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
    }
};

/**
 * VFX Campfire System - Manages realistic campfire visual and audio effects
 * Integrates with Audio_CampfireSystem for synchronized audio-visual experience
 */
UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_CampfireSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_CampfireSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CAMPFIRE STATE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire State")
    EVFX_CampfireState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire State")
    float BurnDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire State")
    float CurrentBurnTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Campfire State")
    bool bIsLit;

    // === VFX COMPONENTS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effects")
    FVFX_CampfireEffects EffectAssets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FlameComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* SmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* EmberComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* SparkComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UPointLightComponent* FireLight;

    // === EFFECT PARAMETERS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float FlameIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters", meta = (ClampMin = "0.1", ClampMax = "3.0"))
    float SmokeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float EmberSpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SparkFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect Parameters")
    FVector WindDirection;

public:
    // === CAMPFIRE CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Campfire Control")
    void LightCampfire();

    UFUNCTION(BlueprintCallable, Category = "Campfire Control")
    void ExtinguishCampfire();

    UFUNCTION(BlueprintCallable, Category = "Campfire Control")
    void AddFuel(float FuelAmount);

    UFUNCTION(BlueprintCallable, Category = "Campfire Control")
    void SetWindEffect(FVector NewWindDirection, float NewWindStrength);

    // === STATE QUERIES ===
    UFUNCTION(BlueprintPure, Category = "Campfire State")
    EVFX_CampfireState GetCampfireState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Campfire State")
    bool IsLit() const { return bIsLit; }

    UFUNCTION(BlueprintPure, Category = "Campfire State")
    float GetBurnTimeRemaining() const { return FMath::Max(0.0f, BurnDuration - CurrentBurnTime); }

    UFUNCTION(BlueprintPure, Category = "Campfire State")
    float GetBurnPercentage() const { return BurnDuration > 0.0f ? (CurrentBurnTime / BurnDuration) : 0.0f; }

    // === EFFECT CONTROL ===
    UFUNCTION(BlueprintCallable, Category = "Effect Control")
    void SetFlameIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Effect Control")
    void SetSmokeIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Effect Control")
    void SetEmberSpawnRate(float NewRate);

    UFUNCTION(BlueprintCallable, Category = "Effect Control")
    void TriggerSparkBurst(int32 SparkCount = 10);

private:
    // === INTERNAL METHODS ===
    void InitializeVFXComponents();
    void UpdateCampfireState(float DeltaTime);
    void UpdateEffectIntensities();
    void UpdateLighting();
    void TransitionToState(EVFX_CampfireState NewState);
    void SpawnRandomSparks();

    // === TIMERS ===
    float SparkTimer;
    float StateTransitionTimer;
    float LightFlickerTimer;

    // === CONSTANTS ===
    static constexpr float IGNITION_TIME = 3.0f;
    static constexpr float DYING_TIME = 10.0f;
    static constexpr float SPARK_INTERVAL_MIN = 2.0f;
    static constexpr float SPARK_INTERVAL_MAX = 8.0f;
    static constexpr float LIGHT_FLICKER_SPEED = 0.5f;
};