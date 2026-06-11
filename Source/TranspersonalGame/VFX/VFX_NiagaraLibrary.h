#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "SharedTypes.h"
#include "VFX_NiagaraLibrary.generated.h"

/**
 * VFX Niagara Library - Centralized VFX management system for prehistoric survival game
 * Handles particle effects for fire, weather, impacts, and environmental effects
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FVFX_ParticleConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TSoftObjectPtr<UNiagaraSystem> ParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    bool bAutoDestroy;

    FVFX_ParticleConfig()
    {
        Scale = FVector(1.0f, 1.0f, 1.0f);
        Intensity = 1.0f;
        Duration = 5.0f;
        bAutoDestroy = true;
    }
};

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Fire_Campfire       UMETA(DisplayName = "Campfire"),
    Fire_Torch          UMETA(DisplayName = "Torch"),
    Impact_Footstep     UMETA(DisplayName = "Footstep Dust"),
    Impact_Blood        UMETA(DisplayName = "Blood Splatter"),
    Impact_Rock         UMETA(DisplayName = "Rock Impact"),
    Weather_Rain        UMETA(DisplayName = "Rain"),
    Weather_Snow        UMETA(DisplayName = "Snow"),
    Weather_Fog         UMETA(DisplayName = "Fog"),
    Environment_Waterfall UMETA(DisplayName = "Waterfall Spray"),
    Environment_Volcano   UMETA(DisplayName = "Volcanic Smoke"),
    Dino_Breath         UMETA(DisplayName = "Dinosaur Breath"),
    Dino_Roar           UMETA(DisplayName = "Roar Distortion")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVFX_NiagaraLibrary : public UActorComponent
{
    GENERATED_BODY()

public:
    UVFX_NiagaraLibrary();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core VFX spawning functions
    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAtLocation(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    UNiagaraComponent* SpawnVFXAttached(EVFX_EffectType EffectType, USceneComponent* AttachToComponent, FName AttachPointName = NAME_None);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopVFX(UNiagaraComponent* VFXComponent);

    UFUNCTION(BlueprintCallable, Category = "VFX")
    void StopAllVFX();

    // Specialized VFX functions for game mechanics
    UFUNCTION(BlueprintCallable, Category = "VFX|Fire")
    UNiagaraComponent* CreateCampfire(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void PlayFootstepDust(FVector Location, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "VFX|Impact")
    void PlayBloodImpact(FVector Location, FVector ImpactNormal, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Weather")
    UNiagaraComponent* StartWeatherEffect(EVFX_EffectType WeatherType, FVector Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX|Dinosaur")
    UNiagaraComponent* PlayDinosaurBreath(FVector Location, FVector Direction, EDinosaurSpecies Species);

    UFUNCTION(BlueprintCallable, Category = "VFX|Environment")
    UNiagaraComponent* CreateEnvironmentalVFX(EVFX_EffectType EnvironmentType, FVector Location, FVector Scale = FVector(1.0f));

    // VFX configuration and management
    UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    void SetVFXIntensity(UNiagaraComponent* VFXComponent, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "VFX|Config")
    void SetVFXScale(UNiagaraComponent* VFXComponent, FVector NewScale);

    UFUNCTION(BlueprintCallable, Category = "VFX|Validation")
    int32 GetActiveVFXCount() const;

    UFUNCTION(BlueprintCallable, Category = "VFX|Validation")
    TArray<UNiagaraComponent*> GetActiveVFXComponents() const;

protected:
    // VFX configuration maps
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Config")
    TMap<EVFX_EffectType, FVFX_ParticleConfig> VFXConfigurations;

    // Active VFX tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX State")
    TArray<UNiagaraComponent*> ActiveVFXComponents;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    int32 MaxActiveVFX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    float VFXCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Performance")
    bool bEnableVFXLOD;

private:
    // Internal helper functions
    UNiagaraSystem* GetVFXSystemForType(EVFX_EffectType EffectType);
    void CleanupInactiveVFX();
    void InitializeVFXConfigurations();
    bool ShouldCullVFX(const FVector& VFXLocation) const;
};