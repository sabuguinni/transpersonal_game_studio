#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/TriggerVolume.h"
#include "SharedTypes.h"
#include "VFX_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EVFX_EffectType : uint8
{
    Footstep_Small      UMETA(DisplayName = "Small Footstep"),
    Footstep_Large      UMETA(DisplayName = "Large Footstep"),
    Dust_Trail          UMETA(DisplayName = "Dust Trail"),
    Atmosphere_Tension  UMETA(DisplayName = "Atmospheric Tension"),
    Impact_Ground       UMETA(DisplayName = "Ground Impact"),
    Danger_Zone         UMETA(DisplayName = "Danger Zone Particles"),
    Water_Splash        UMETA(DisplayName = "Water Splash"),
    Blood_Impact        UMETA(DisplayName = "Blood Impact")
};

USTRUCT(BlueprintType)
struct FVFX_EffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    EVFX_EffectType EffectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    TSoftObjectPtr<UNiagaraSystem> NiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Effect")
    bool bAutoDestroy;

    FVFX_EffectData()
    {
        EffectType = EVFX_EffectType::Footstep_Small;
        Duration = 2.0f;
        Intensity = 1.0f;
        bAutoDestroy = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UVFX_SystemManager : public UObject
{
    GENERATED_BODY()

public:
    UVFX_SystemManager();

    // Core VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void InitializeVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnEffect(EVFX_EffectType EffectType, FVector Location, FRotator Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnFootstepEffect(FVector Location, bool bIsLargeDinosaur = false);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnAtmosphericTension(FVector Location, float TensionLevel = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnDustTrail(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "VFX System")
    void SpawnDangerZoneParticles(FVector Location, float Radius = 500.0f);

    // Environmental VFX
    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void UpdateEnvironmentalEffects(EBiomeType BiomeType, float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environmental VFX")
    void SetWeatherEffects(EWeatherType WeatherType, float Intensity);

    // Combat VFX
    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void SpawnImpactEffect(FVector Location, EVFX_EffectType ImpactType);

    UFUNCTION(BlueprintCallable, Category = "Combat VFX")
    void SpawnBloodEffect(FVector Location, FVector Direction);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVFXQualityLevel(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupExpiredEffects();

protected:
    // VFX Effect Registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Registry")
    TMap<EVFX_EffectType, FVFX_EffectData> EffectRegistry;

    // Active Effect Tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Active Effects")
    TArray<UNiagaraComponent*> ActiveEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EffectCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 CurrentQualityLevel;

private:
    // Internal Methods
    void LoadEffectAssets();
    void RegisterDefaultEffects();
    UNiagaraComponent* CreateNiagaraComponent(UNiagaraSystem* System, FVector Location, FRotator Rotation);
    void ManageEffectPool();
    bool ShouldCullEffect(const UNiagaraComponent* Effect, FVector PlayerLocation) const;

    // Effect Pool Management
    TArray<UNiagaraComponent*> EffectPool;
    int32 PoolIndex;
    
    // Timing
    float LastCleanupTime;
    const float CleanupInterval = 5.0f;
};