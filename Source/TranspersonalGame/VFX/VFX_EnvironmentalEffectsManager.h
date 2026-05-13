#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "VFX_EnvironmentalEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_WeatherType : uint8
{
    Clear       UMETA(DisplayName = "Clear Weather"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Thunderstorm"), 
    Fog         UMETA(DisplayName = "Dense Fog"),
    Dust        UMETA(DisplayName = "Dust Storm")
};

UENUM(BlueprintType)
enum class EVFX_VolcanicIntensity : uint8
{
    Dormant     UMETA(DisplayName = "Dormant"),
    Smoking     UMETA(DisplayName = "Light Smoke"),
    Active      UMETA(DisplayName = "Active Eruption"),
    Explosive   UMETA(DisplayName = "Major Eruption")
};

USTRUCT(BlueprintType)
struct FVFX_WeatherSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EVFX_WeatherType WeatherType = EVFX_WeatherType::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float Duration = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectsVisibility = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectsMovement = false;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AVFX_EnvironmentalEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Weather System Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* RainVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* FogVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* DustStormVFXComponent;

    // Volcanic System Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* VolcanicSmokeVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* LavaVFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UNiagaraComponent* AshVFXComponent;

public:
    // Weather Control Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWeatherType(EVFX_WeatherType NewWeatherType, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void StartWeatherTransition(const FVFX_WeatherSettings& NewWeatherSettings, float TransitionDuration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetVolcanicActivity(EVFX_VolcanicIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void TriggerVolcanicEruption(float Duration = 60.0f);

    // Environmental Hazard Functions
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void CreateDustCloud(FVector Location, float Radius = 500.0f, float Duration = 15.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void CreateWaterSplash(FVector Location, float Scale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void CreateCampfireEffect(FVector Location, bool bIncludeSmoke = true);

protected:
    // Current weather state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental State")
    FVFX_WeatherSettings CurrentWeatherSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental State")
    EVFX_VolcanicIntensity CurrentVolcanicIntensity = EVFX_VolcanicIntensity::Dormant;

    // Weather transition
    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float TransitionProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float TransitionDuration = 10.0f;

    FVFX_WeatherSettings TargetWeatherSettings;

    // Volcanic eruption state
    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    bool bVolcanicEruptionActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Environmental State")
    float EruptionTimeRemaining = 0.0f;

private:
    void UpdateWeatherTransition(float DeltaTime);
    void UpdateVolcanicEffects(float DeltaTime);
    void ApplyWeatherIntensity(float Intensity);
    void ApplyVolcanicIntensity(EVFX_VolcanicIntensity Intensity);
};