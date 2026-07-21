#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Perf_WalkAroundFrameRateController.generated.h"

UENUM(BlueprintType)
enum class EPerf_FrameRateTarget : uint8
{
    Target_30FPS    UMETA(DisplayName = "30 FPS (Console)"),
    Target_60FPS    UMETA(DisplayName = "60 FPS (PC Standard)"),
    Target_120FPS   UMETA(DisplayName = "120 FPS (High-End PC)"),
    Target_Variable UMETA(DisplayName = "Variable (Adaptive)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_FrameRateMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MinFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DroppedFrames = 0;

    FPerf_FrameRateMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        MinFPS = 999.0f;
        MaxFPS = 0.0f;
        FrameTime = 0.0f;
        GameThreadTime = 0.0f;
        RenderThreadTime = 0.0f;
        DroppedFrames = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_WalkAroundOptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Performance")
    float CharacterCullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Performance")
    int32 MaxActiveCharacters = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    float DinosaurCullingDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Performance")
    int32 MaxActiveDinosaurs = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Performance")
    float FoliageCullingDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Performance")
    int32 MaxFoliageInstances = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering Performance")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rendering Performance")
    float LODDistanceMultiplier = 1.0f;

    FPerf_WalkAroundOptimizationSettings()
    {
        CharacterCullingDistance = 5000.0f;
        MaxActiveCharacters = 10;
        DinosaurCullingDistance = 8000.0f;
        MaxActiveDinosaurs = 5;
        FoliageCullingDistance = 3000.0f;
        MaxFoliageInstances = 1000;
        bEnablePhysicsOptimization = true;
        PhysicsTickRate = 60.0f;
        bEnableLODOptimization = true;
        LODDistanceMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_WalkAroundFrameRateController : public AActor
{
    GENERATED_BODY()

public:
    APerf_WalkAroundFrameRateController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate Control")
    EPerf_FrameRateTarget TargetFrameRate = EPerf_FrameRateTarget::Target_60FPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate Control")
    bool bEnableAdaptiveFrameRate = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Frame Rate Control")
    float FrameRateThreshold = 55.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_FrameRateMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization Settings")
    FPerf_WalkAroundOptimizationSettings OptimizationSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    bool bEnablePerformanceLogging = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Monitoring")
    float LoggingInterval = 1.0f;

private:
    float LastLogTime = 0.0f;
    TArray<float> FPSHistory;
    int32 FrameCounter = 0;
    float TotalFrameTime = 0.0f;

    void UpdateFrameRateMetrics(float DeltaTime);
    void ApplyOptimizations();
    void LogPerformanceMetrics();

public:
    UFUNCTION(BlueprintCallable, Category = "Performance Control")
    void SetTargetFrameRate(EPerf_FrameRateTarget NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Performance Control")
    void EnableAdaptiveFrameRate(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance Control")
    FPerf_FrameRateMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Control")
    void ResetMetrics();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void ApplyWalkAroundOptimizations();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void SetOptimizationLevel(EOptimizationLevel Level);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestFrameRateControl();
};