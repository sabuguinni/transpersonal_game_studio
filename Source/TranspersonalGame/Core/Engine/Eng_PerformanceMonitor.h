#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Eng_PerformanceMonitor.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VisibleActors = 0;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MinFPS = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxFrameTime = 33.33f; // 30 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxTriangles = 1000000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MaxMemoryMB = 4096.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 MaxActors = 8000;
};

UENUM(BlueprintType)
enum class EEng_PerformanceLevel : uint8
{
    Excellent   UMETA(DisplayName = "Excellent"),
    Good        UMETA(DisplayName = "Good"),
    Acceptable  UMETA(DisplayName = "Acceptable"),
    Poor        UMETA(DisplayName = "Poor"),
    Critical    UMETA(DisplayName = "Critical")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PerformanceMonitor : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_PerformanceMonitor();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    FEng_PerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    EEng_PerformanceLevel GetPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void SetPerformanceThresholds(const FEng_PerformanceThresholds& Thresholds);

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StartPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void StopPerformanceCapture();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    TArray<FString> GetPerformanceWarnings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor", CallInEditor = true)
    void LogPerformanceReport();

    UFUNCTION(BlueprintCallable, Category = "Performance Monitor")
    void OptimizeForTargetFPS(float TargetFPS);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEng_PerformanceThresholds Thresholds;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FEng_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bLogPerformanceWarnings = true;

private:
    float TimeSinceLastUpdate = 0.0f;
    TArray<float> FPSHistory;
    bool bIsCapturing = false;
    
    void UpdateMetrics();
    void CheckPerformanceThresholds();
    void ApplyAutoOptimizations();
    float CalculateAverageFPS() const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_PerformanceDebugActor : public AActor
{
    GENERATED_BODY()

public:
    AEng_PerformanceDebugActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Performance Debug")
    void TogglePerformanceDisplay();

    UFUNCTION(BlueprintCallable, Category = "Performance Debug")
    void ShowDetailedStats();

    UFUNCTION(BlueprintCallable, Category = "Performance Debug")
    void HideDetailedStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bShowPerformanceHUD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FVector2D HUDPosition = FVector2D(50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float UpdateFrequency = 0.5f;

private:
    UEng_PerformanceMonitor* PerformanceMonitor = nullptr;
    float LastUpdateTime = 0.0f;
    
    void DrawPerformanceHUD();
};