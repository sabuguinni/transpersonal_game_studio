#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "VFX_EnvironmentalController.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    VolcanicSmoke    UMETA(DisplayName = "Volcanic Smoke"),
    WaterfallMist    UMETA(DisplayName = "Waterfall Mist"),
    WindDust         UMETA(DisplayName = "Wind Dust"),
    InsectSwarm      UMETA(DisplayName = "Insect Swarm"),
    PollenDrift      UMETA(DisplayName = "Pollen Drift"),
    AshFall          UMETA(DisplayName = "Volcanic Ash"),
    SteamVent        UMETA(DisplayName = "Steam Vent"),
    Fireflies        UMETA(DisplayName = "Fireflies")
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    EVFX_EnvironmentalType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float EffectIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float EffectRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    bool bIsTimeOfDayDependent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float MinTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    float MaxTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental VFX")
    bool bIsWeatherDependent;

    FVFX_EnvironmentalSettings()
    {
        EffectType = EVFX_EnvironmentalType::WindDust;
        EffectIntensity = 1.0f;
        EffectRadius = 500.0f;
        bIsTimeOfDayDependent = false;
        MinTimeOfDay = 0.0f;
        MaxTimeOfDay = 24.0f;
        bIsWeatherDependent = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EnvironmentalController : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UBoxComponent* EffectTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    UNiagaraComponent* PrimaryEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    FVFX_EnvironmentalSettings EnvironmentalSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    bool bAutoActivate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Settings")
    float UpdateFrequency;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bIsEffectActive;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float CurrentIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float LastUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void ActivateEnvironmentalEffect();

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void DeactivateEnvironmentalEffect();

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateEffectForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateEffectForWeather(float WeatherIntensity, int32 WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    bool ShouldEffectBeActive() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental VFX")
    void OnEffectActivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental VFX")
    void OnEffectDeactivated();

    UFUNCTION(BlueprintImplementableEvent, Category = "Environmental VFX")
    void OnIntensityChanged(float NewIntensity);

private:
    void InitializeEffectComponent();
    void UpdateEffectParameters();
    float CalculateIntensityForConditions() const;
};