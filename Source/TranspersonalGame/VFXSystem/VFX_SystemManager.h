#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "VFX_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    None            UMETA(DisplayName = "None"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    Campfire        UMETA(DisplayName = "Campfire"),
    BloodImpact     UMETA(DisplayName = "Blood Impact"),
    WeatherRain     UMETA(DisplayName = "Weather Rain"),
    DinoBreath      UMETA(DisplayName = "Dinosaur Breath"),
    WeaponImpact    UMETA(DisplayName = "Weapon Impact"),
    EnvironmentalFog UMETA(DisplayName = "Environmental Fog")
};

USTRUCT(BlueprintType)
struct FVFX_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EffectType EffectType = EVFX_EffectType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Duration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoDestroy = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float FadeOutTime = 1.0f;

    FVFX_EffectSettings()
    {
        EffectType = EVFX_EffectType::None;
        Duration = 2.0f;
        Intensity = 1.0f;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        bAutoDestroy = true;
        FadeOutTime = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // VFX Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* FootstepDustComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* CampfireComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* BloodImpactComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* WeatherRainComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* DinoBreathComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Components")
    class UParticleSystemComponent* WeaponImpactComponent;

public:
    // VFX Control Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerFootstepDust(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopCampfire();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerBloodImpact(FVector Location, FVector Direction, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StartWeatherRain(float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopWeatherRain();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerDinoBreath(FVector Location, FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerWeaponImpact(FVector Location, FVector Direction, float Damage = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void PlayEffect(EVFX_EffectType EffectType, FVector Location, const FVFX_EffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void StopAllEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetGlobalVFXIntensity(float NewIntensity);

protected:
    // VFX Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bVFXEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    float MaxVFXDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    int32 MaxActiveEffects = 50;

    // Effect Tracking
    UPROPERTY()
    TArray<class UParticleSystemComponent*> ActiveEffects;

    UPROPERTY()
    FTimerHandle CleanupTimerHandle;

private:
    // Internal Functions
    void InitializeVFXComponents();
    void CleanupFinishedEffects();
    bool ShouldPlayEffect(FVector Location) const;
    UParticleSystemComponent* CreateEffectComponent(EVFX_EffectType EffectType);
    void ConfigureEffectComponent(UParticleSystemComponent* Component, const FVFX_EffectSettings& Settings);
};