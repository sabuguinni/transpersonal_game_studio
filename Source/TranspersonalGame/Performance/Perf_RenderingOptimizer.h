#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_RenderingOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RenderQuality : uint8
{
    Low UMETA(DisplayName = "Low"),
    Medium UMETA(DisplayName = "Medium"),
    High UMETA(DisplayName = "High"),
    Ultra UMETA(DisplayName = "Ultra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    EPerf_RenderQuality QualityLevel = EPerf_RenderQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float RenderScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering")
    bool bEnableOcclusion = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering", meta = (ClampMin = "500", ClampMax = "10000"))
    float MaxDrawDistance = 5000.0f;

    FPerf_RenderingSettings()
    {
        QualityLevel = EPerf_RenderQuality::High;
        RenderScale = 1.0f;
        bEnableDynamicLOD = true;
        bEnableOcclusion = true;
        MaxDrawDistance = 5000.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RenderingOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RenderingOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_RenderingSettings RenderingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (ClampMin = "30", ClampMax = "120"))
    int32 TargetFPS = 60;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bAutoOptimize = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings", meta = (ClampMin = "1.0", ClampMax = "10.0"))
    float OptimizationInterval = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float AverageFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 VisibleActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 CulledActors = 0;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeRendering();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderQuality(EPerf_RenderQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetRenderScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableLODOptimization(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxDrawDistance(float Distance);

    UFUNCTION(BlueprintPure, Category = "Performance")
    float GetCurrentFPS() const { return CurrentFPS; }

    UFUNCTION(BlueprintPure, Category = "Performance")
    bool IsPerformanceGood() const { return CurrentFPS >= (TargetFPS * 0.9f); }

private:
    void UpdatePerformanceStats();
    void ApplyRenderingOptimizations();
    void OptimizeMeshComponents();
    void UpdateLODSettings();
    void CullDistantObjects();

    float LastOptimizationTime = 0.0f;
    TArray<float> FrameTimeHistory;
    int32 MaxFrameHistorySize = 60;
};