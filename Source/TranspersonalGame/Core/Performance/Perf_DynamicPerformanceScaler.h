#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_DynamicPerformanceScaler.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"),
    High        UMETA(DisplayName = "High Performance"),
    Ultra       UMETA(DisplayName = "Ultra Performance")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ViewDistanceScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ShadowQuality = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 FoliageQuality = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float ScreenPercentage = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LODBias = 0;

    FPerf_PerformanceSettings()
    {
        ViewDistanceScale = 1.0f;
        ShadowQuality = 4;
        FoliageQuality = 4;
        ScreenPercentage = 100.0f;
        LODBias = 0;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_DynamicPerformanceScaler : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_DynamicPerformanceScaler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnableAutoScaling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float MinFrameRate = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    int32 ActorCountThreshold = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float PerformanceCheckInterval = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    EPerf_PerformanceLevel CurrentPerformanceLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    float CurrentFrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Status")
    int32 CurrentActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PerformanceSettings LowSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PerformanceSettings MediumSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PerformanceSettings HighSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_PerformanceSettings UltraSettings;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceLevel(EPerf_PerformanceLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AnalyzeCurrentPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForActorCount(int32 ActorCount);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForFrameRate(float FrameRate);

    UFUNCTION(BlueprintPure, Category = "Performance")
    FString GetPerformanceReport() const;

private:
    float LastPerformanceCheck;
    float FrameRateHistory[10];
    int32 FrameRateHistoryIndex;
    
    void InitializePerformanceSettings();
    void ApplyPerformanceSettings(const FPerf_PerformanceSettings& Settings);
    float CalculateAverageFrameRate() const;
    void UpdateFrameRateHistory(float NewFrameRate);
};