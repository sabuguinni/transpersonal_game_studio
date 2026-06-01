#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "VFX_NiagaraManager.generated.h"

/**
 * VFX Niagara Manager - Controls prehistoric VFX systems
 * Handles dust impacts, campfires, weather effects, and blood splatter
 * All effects are realistic and appropriate for prehistoric survival setting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVFX_NiagaraManager : public AActor
{
    GENERATED_BODY()

public:
    AVFX_NiagaraManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VFX Core")
    USceneComponent* RootSceneComponent;

    // VFX System Categories
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    bool bEnableDustEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    bool bEnableFireEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    bool bEnableWeatherEffects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Systems")
    bool bEnableBloodEffects;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float VFXQualityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float MaxVFXDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveVFXSystems;

    // Biome-specific VFX intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float SavanaDustIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float SwampMistIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float ForestParticleIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float DesertSandstormIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome VFX")
    float MountainSnowIntensity;

public:
    // VFX Trigger Functions
    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerFootstepDust(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerBloodImpact(FVector Location, FVector Direction, float Amount = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void CreateCampfire(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "VFX Triggers")
    void TriggerWeatherEffect(EVFX_WeatherType WeatherType, FVector Location, float Intensity = 1.0f);

    // VFX Management
    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void SetVFXQuality(float NewQuality);

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    void CleanupDistantVFX();

    UFUNCTION(BlueprintCallable, Category = "VFX Management")
    int32 GetActiveVFXCount() const;

    // Biome Detection
    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    EVFX_BiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    float GetBiomeVFXIntensity(EVFX_BiomeType BiomeType) const;

protected:
    // Internal VFX tracking
    UPROPERTY()
    TArray<AActor*> ActiveVFXActors;

    UPROPERTY()
    float LastCleanupTime;

    // Helper functions
    void InitializeVFXSystems();
    void UpdateVFXPerformance();
    bool IsLocationValid(FVector Location) const;
    float CalculateDistanceToPlayer(FVector Location) const;
};