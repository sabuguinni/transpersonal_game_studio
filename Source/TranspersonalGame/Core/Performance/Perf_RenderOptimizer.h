#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_RenderOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RenderQuality : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct FPerf_RenderSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    EPerf_RenderQuality QualityLevel = EPerf_RenderQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    float ShadowDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    bool bEnableLumen = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    bool bEnableNanite = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Settings")
    int32 MaxFoliageInstances = 10000;

    FPerf_RenderSettings()
    {
        QualityLevel = EPerf_RenderQuality::High;
        ViewDistanceScale = 1.0f;
        ShadowDistanceScale = 1.0f;
        bEnableLumen = true;
        bEnableNanite = true;
        MaxFoliageInstances = 10000;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RenderOptimizer();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_RenderSettings RenderSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float AdaptationSpeed = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameRate = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bIsOptimizing = false;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRenderSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderQuality(EPerf_RenderQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdaptToFrameRate(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyRenderSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ResetToDefaults();

private:
    void UpdateFrameRateHistory(float FrameRate);
    void OptimizeViewDistance();
    void OptimizeShadowSettings();
    void OptimizeFoliageSettings();
    void OptimizeLightingSettings();

    TArray<float> FrameRateHistory;
    int32 MaxHistorySize = 60;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 2.0f;
};