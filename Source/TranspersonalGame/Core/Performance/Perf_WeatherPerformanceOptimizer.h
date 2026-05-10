#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Niagara/Public/NiagaraSystem.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Perf_WeatherPerformanceOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_WeatherPerformanceLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Minimal     UMETA(DisplayName = "Minimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WeatherPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    int32 MaxRainParticles = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    int32 MaxFogVolumes = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float WeatherUpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    bool bEnableWeatherShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float MaxWeatherDrawDistance = 50000.0f;

    FPerf_WeatherPerformanceSettings()
    {
        MaxRainParticles = 10000;
        MaxFogVolumes = 5;
        WeatherUpdateFrequency = 1.0f;
        bEnableVolumetricFog = true;
        bEnableWeatherShadows = true;
        MaxWeatherDrawDistance = 50000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance))
class TRANSPERSONALGAME_API UPerf_WeatherPerformanceOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_WeatherPerformanceOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void SetWeatherPerformanceLevel(EPerf_WeatherPerformanceLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void UpdateWeatherLOD(float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeRainSystem(class UNiagaraComponent* RainComponent);

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void OptimizeFogSystem();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    float GetWeatherPerformanceImpact() const;

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void CullDistantWeatherEffects();

    UFUNCTION(BlueprintCallable, Category = "Weather Performance")
    void AdjustWeatherQuality(float FrameTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    EPerf_WeatherPerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    FPerf_WeatherPerformanceSettings PerformanceSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float LastPerformanceUpdate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    TArray<TWeakObjectPtr<UNiagaraComponent>> TrackedWeatherComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    float WeatherPerformanceBudget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Performance")
    bool bAutoAdjustQuality;

private:
    void InitializePerformanceSettings();
    void UpdatePerformanceMetrics();
    FPerf_WeatherPerformanceSettings GetSettingsForLevel(EPerf_WeatherPerformanceLevel Level);
    void ApplyPerformanceSettings(const FPerf_WeatherPerformanceSettings& Settings);
    void RegisterWeatherComponent(UNiagaraComponent* Component);
    void UnregisterWeatherComponent(UNiagaraComponent* Component);
    void CleanupInvalidComponents();
};