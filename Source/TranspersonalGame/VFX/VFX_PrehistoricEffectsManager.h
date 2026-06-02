#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "VFX_PrehistoricEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Dust_FootstepImpact UMETA(DisplayName = "Footstep Dust"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Combat_BloodSplash  UMETA(DisplayName = "Blood Splash"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Environment_Insects UMETA(DisplayName = "Insects"),
    Volcanic_Ash        UMETA(DisplayName = "Volcanic Ash")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EffectType EffectType = EVFX_EffectType::Fire_Campfire;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector Scale = FVector(1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float IntensityMultiplier = 1.0f;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Fire_Campfire;
        Scale = FVector(1.0f);
        Duration = 5.0f;
        bLooping = false;
        IntensityMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_PrehistoricEffectsManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_PrehistoricEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    TArray<FVFX_EffectData> AvailableEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Systems")
    TArray<UNiagaraComponent*> ActiveEffectComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bEnableCombatEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float EffectCullingDistance = 5000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator, float CustomDuration = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnCampfireEffect(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnFootstepDustEffect(FVector Location, float ImpactStrength = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnBloodSplashEffect(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SpawnWaterSplashEffect(FVector Location, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StartWeatherEffect(EVFX_EffectType WeatherType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopWeatherEffect(EVFX_EffectType WeatherType);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void SetGlobalIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void CleanupExpiredEffects();

protected:
    UFUNCTION()
    void InitializeEffectLibrary();

    UFUNCTION()
    UNiagaraComponent* CreateEffectComponent(const FVFX_EffectData& EffectData, FVector Location, FRotator Rotation);

    UFUNCTION()
    void UpdateEffectLOD();

    FTimerHandle EffectCleanupTimer;
    FTimerHandle LODUpdateTimer;
};