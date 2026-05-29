#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "../SharedTypes.h"
#include "VFX_WeatherParticleController.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear           UMETA(DisplayName = "Clear Sky"),
    LightRain       UMETA(DisplayName = "Light Rain"),
    HeavyRain       UMETA(DisplayName = "Heavy Rain"),
    Thunderstorm    UMETA(DisplayName = "Thunderstorm"),
    WindStorm       UMETA(DisplayName = "Wind Storm"),
    Fog             UMETA(DisplayName = "Fog"),
    Sandstorm       UMETA(DisplayName = "Sandstorm")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_WeatherParticleSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float RainIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float FogDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float LightningFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float ParticleLifetime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    int32 MaxParticleCount = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather VFX")
    float VisibilityRange = 5000.0f;

    FVFX_WeatherParticleSettings()
    {
        RainIntensity = 1.0f;
        WindStrength = 1.0f;
        FogDensity = 0.5f;
        LightningFrequency = 0.1f;
        WindDirection = FVector(1.0f, 0.0f, 0.0f);
        ParticleLifetime = 5.0f;
        MaxParticleCount = 10000;
        VisibilityRange = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_LightningStrike
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    FVector StrikeLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float Duration = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float ThunderDelay = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    bool bCausesScreenFlash = true;

    FVFX_LightningStrike()
    {
        StrikeLocation = FVector::ZeroVector;
        Intensity = 1.0f;
        Duration = 0.2f;
        ThunderDelay = 3.0f;
        bCausesScreenFlash = true;
    }
};

/**
 * VFX_WeatherParticleController - Manages realistic Cretaceous period weather particle effects
 * Handles rain, lightning, fog, wind, and atmospheric effects with performance optimization
 * Syncs with Audio_WeatherAudioSystem for immersive weather experiences
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_WeatherParticleController : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_WeatherParticleController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void SetWeatherType(EVFX_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void UpdateWeatherIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void TriggerLightningStrike(const FVFX_LightningStrike& LightningData);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void SetWindDirection(const FVector& NewWindDirection);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void UpdateFogDensity(float NewDensity);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void SetLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Weather VFX")
    void EnablePerformanceMode(bool bEnabled);

    // Weather State Queries
    UFUNCTION(BlueprintPure, Category = "Weather VFX")
    EVFX_WeatherType GetCurrentWeatherType() const { return CurrentWeatherType; }

    UFUNCTION(BlueprintPure, Category = "Weather VFX")
    float GetWeatherIntensity() const { return WeatherIntensity; }

    UFUNCTION(BlueprintPure, Category = "Weather VFX")
    bool IsRaining() const;

    UFUNCTION(BlueprintPure, Category = "Weather VFX")
    bool IsStorming() const;

protected:
    // Weather System Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    EVFX_WeatherType CurrentWeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    float WeatherIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Settings")
    FVFX_WeatherParticleSettings ParticleSettings;

    // Niagara System References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* LightningSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* FogSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* WindParticleSystem;

    // Active Niagara Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* ActiveRainComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* ActiveLightningComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* ActiveFogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* ActiveWindComponent;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceModeEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveParticles = 50000;

    // Lightning System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    TArray<FVFX_LightningStrike> PendingLightningStrikes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float LightningCooldown = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lightning")
    float MinLightningInterval = 2.0f;

private:
    // Internal Weather Management
    void InitializeWeatherSystems();
    void UpdateActiveWeatherEffects(float DeltaTime);
    void ProcessLightningStrikes(float DeltaTime);
    void UpdateParticleParameters();
    void OptimizePerformance();
    
    // Weather Transition Functions
    void TransitionToWeatherType(EVFX_WeatherType NewType);
    void BlendWeatherEffects(float BlendAlpha);
    
    // Utility Functions
    FVector GetPlayerLocation() const;
    float GetDistanceToPlayer() const;
    bool ShouldRenderWeatherEffect() const;
};