#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_EnvironmentalEffects.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Rain        UMETA(DisplayName = "Rain"),
    Mist        UMETA(DisplayName = "Mist"),
    Dust        UMETA(DisplayName = "Dust Storm"),
    VolcanicAsh UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float WindStrength = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bIsActive = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EnvironmentalEffects : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalEffects();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* WeatherEffect;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* AmbientParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_WeatherType CurrentWeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVFX_EnvironmentalSettings EnvironmentalSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* MistSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* DustSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Assets")
    class UNiagaraSystem* VolcanicAshSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float TransitionDuration = 5.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetWeatherType(EVFX_WeatherType NewWeatherType);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void UpdateEnvironmentalSettings(const FVFX_EnvironmentalSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartWeatherTransition(EVFX_WeatherType TargetWeather);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllEffects();

private:
    void InitializeWeatherSystems();
    void UpdateWeatherEffect();
    void OnTransitionComplete();

    FTimerHandle TransitionTimer;
    EVFX_WeatherType TargetWeatherType;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
};