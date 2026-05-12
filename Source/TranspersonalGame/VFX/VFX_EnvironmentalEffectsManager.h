#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "../SharedTypes.h"
#include "VFX_EnvironmentalEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EnvironmentalType : uint8
{
    DustCloud       UMETA(DisplayName = "Dust Cloud"),
    WaterSplash     UMETA(DisplayName = "Water Splash"),
    RainDroplets    UMETA(DisplayName = "Rain Droplets"),
    VolcanicAsh     UMETA(DisplayName = "Volcanic Ash"),
    WindParticles   UMETA(DisplayName = "Wind Particles"),
    FogMist         UMETA(DisplayName = "Fog Mist")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_EnvironmentalEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    EVFX_EnvironmentalType EffectType = EVFX_EnvironmentalType::DustCloud;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Duration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bAttachToActor = false;

    FVFX_EnvironmentalEffect()
    {
        EffectType = EVFX_EnvironmentalType::DustCloud;
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        Duration = 3.0f;
        Intensity = 1.0f;
        bAttachToActor = false;
    }
};

UCLASS(ClassGroup=(VFX), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_EnvironmentalEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_EnvironmentalEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Environmental VFX spawning
    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnEnvironmentalEffect(const FVFX_EnvironmentalEffect& EffectData);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnDustCloud(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnWaterSplash(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnRainEffect(FVector Location, float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environmental")
    void SpawnVolcanicAsh(FVector Location, float Duration = 30.0f);

    // Weather system integration
    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void UpdateWeatherEffects(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StartRainSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    void StopRainSystem();

    // Cleanup and management
    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void CleanupExpiredEffects();

    UFUNCTION(BlueprintCallable, Category = "VFX|Management")
    void SetGlobalVFXIntensity(float NewIntensity);

protected:
    // VFX asset references
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> DustCloudSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> WaterSplashSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> RainSystem;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VFX|Assets")
    TSoftObjectPtr<UNiagaraSystem> VolcanicAshSystem;

    // Active effect tracking
    UPROPERTY(BlueprintReadOnly, Category = "VFX|Runtime")
    TArray<UNiagaraComponent*> ActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    float GlobalVFXIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    int32 MaxActiveEffects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX|Settings")
    bool bEnableEnvironmentalVFX = true;

private:
    // Internal management
    void InitializeVFXSystems();
    UNiagaraComponent* CreateNiagaraEffect(UNiagaraSystem* System, FVector Location, FRotator Rotation);
    void RemoveExpiredEffect(UNiagaraComponent* Effect);

    // Timer handles for cleanup
    FTimerHandle CleanupTimerHandle;
};