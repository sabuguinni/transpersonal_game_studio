#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "VFX_EnvironmentalEffects.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    VolcanicEruption    UMETA(DisplayName = "Volcanic Eruption"),
    WaterSplash         UMETA(DisplayName = "Water Splash"),
    DustCloud           UMETA(DisplayName = "Dust Cloud"),
    WindEffect          UMETA(DisplayName = "Wind Effect"),
    RainDroplets        UMETA(DisplayName = "Rain Droplets")
};

USTRUCT(BlueprintType)
struct FVFX_EnvironmentalSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    EVFX_EnvironmentalType EffectType = EVFX_EnvironmentalType::DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bAutoActivate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Settings")
    bool bLooping = false;

    FVFX_EnvironmentalSettings()
    {
        EffectType = EVFX_EnvironmentalType::DustCloud;
        Intensity = 1.0f;
        Duration = 5.0f;
        bAutoActivate = true;
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_EnvironmentalEffects : public AActor
{
    GENERATED_BODY()

public:
    AVFX_EnvironmentalEffects();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* PrimaryParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SecondaryParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualReferenceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* EnvironmentalAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    FVFX_EnvironmentalSettings EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TArray<UParticleSystem*> VolcanicParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TArray<UParticleSystem*> WaterSplashParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TArray<UParticleSystem*> DustParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Configuration")
    TArray<USoundBase*> EnvironmentalSounds;

public:
    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void ActivateEnvironmentalEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void DeactivateEnvironmentalEffect();

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetEffectType(EVFX_EnvironmentalType NewType);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void SetEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerVolcanicEruption(FVector EruptionLocation, float EruptionPower = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void TriggerWaterSplash(FVector SplashLocation, float SplashSize = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Control")
    void CreateDustCloud(FVector DustLocation, float CloudRadius = 500.0f);

private:
    void SetupParticleSystemForType(EVFX_EnvironmentalType Type);
    void PlayEnvironmentalSound(EVFX_EnvironmentalType Type);
    void StopAllEffects();

    UFUNCTION()
    void OnEffectDurationComplete();

    FTimerHandle EffectDurationTimer;
    bool bEffectActive = false;
};