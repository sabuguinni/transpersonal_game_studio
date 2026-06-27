#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PerformanceManager.generated.h"

// ============================================================
// Performance budget thresholds for 60fps PC / 30fps Console
// ============================================================

UENUM(BlueprintType)
enum class EPerf_TargetPlatform : uint8
{
    PC_High     UMETA(DisplayName = "PC High-End"),
    PC_Mid      UMETA(DisplayName = "PC Mid-Range"),
    Console     UMETA(DisplayName = "Console")
};

UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Ultra   UMETA(DisplayName = "Ultra"),
    High    UMETA(DisplayName = "High"),
    Medium  UMETA(DisplayName = "Medium"),
    Low     UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct FPerf_FrameBudget
{
    GENERATED_BODY()

    // Target frame time in milliseconds
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float TargetFrameTimeMs = 16.67f; // 60fps

    // Current measured frame time
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFrameTimeMs = 0.0f;

    // GPU time budget (ms)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUBudgetMs = 10.0f;

    // CPU time budget (ms)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CPUBudgetMs = 6.0f;

    // Max draw calls per frame
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 MaxDrawCalls = 2000;

    // Max triangle count (millions)
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MaxTrianglesM = 5.0f;
};

USTRUCT(BlueprintType)
struct FPerf_LODConfig
{
    GENERATED_BODY()

    // Screen size threshold for LOD0 -> LOD1 transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0ScreenSize = 0.15f;

    // Screen size threshold for LOD1 -> LOD2 transition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1ScreenSize = 0.08f;

    // Screen size threshold for LOD2 -> cull
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2ScreenSize = 0.03f;

    // Cull distance in cm
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistanceCm = 8000.0f;
};

USTRUCT(BlueprintType)
struct FPerf_DinosaurBudget
{
    GENERATED_BODY()

    // Max simultaneous dinosaurs with full AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Budget")
    int32 MaxFullAIDinosaurs = 12;

    // Max simultaneous dinosaurs with simplified AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Budget")
    int32 MaxSimplifiedAIDinosaurs = 40;

    // Distance at which AI switches to simplified (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Budget")
    float AISimplifyDistanceCm = 3000.0f;

    // Distance at which dinosaur is fully culled
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Budget")
    float CullDistanceCm = 10000.0f;
};

/**
 * UPerf_PerformanceManager
 * GameInstance subsystem that monitors and enforces frame budget.
 * Targets: 60fps PC High-End / 30fps Console
 */
UCLASS(BlueprintType, DisplayName = "Performance Manager")
class TRANSPERSONALGAME_API UPerf_PerformanceManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Apply quality preset for target platform
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyQualityPreset(EPerf_TargetPlatform Platform, EPerf_QualityLevel Quality);

    // Get current frame budget status
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_FrameBudget GetFrameBudget() const;

    // Get LOD configuration
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_LODConfig GetLODConfig() const;

    // Get dinosaur budget
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    FPerf_DinosaurBudget GetDinosaurBudget() const;

    // Check if we are over budget (returns true if frame time exceeds target)
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Performance")
    bool IsOverBudget() const;

    // Force LOD bias adjustment (positive = lower quality, negative = higher)
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODBias(float Bias);

    // Enable/disable expensive features dynamically
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLumenEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVirtualShadowMapsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetNaniteEnabled(bool bEnabled);

    // Tick: sample frame time and auto-adjust if over budget
    void Tick(float DeltaTime);

private:
    UPROPERTY()
    FPerf_FrameBudget FrameBudget;

    UPROPERTY()
    FPerf_LODConfig LODConfig;

    UPROPERTY()
    FPerf_DinosaurBudget DinosaurBudget;

    UPROPERTY()
    EPerf_TargetPlatform CurrentPlatform = EPerf_TargetPlatform::PC_High;

    UPROPERTY()
    EPerf_QualityLevel CurrentQuality = EPerf_QualityLevel::High;

    // Rolling average of last N frame times
    TArray<float> FrameTimeSamples;
    int32 FrameSampleIndex = 0;
    static constexpr int32 FrameSampleCount = 60;

    // Auto-adjustment state
    float TimeSinceLastAdjustment = 0.0f;
    float AdjustmentCooldownSeconds = 5.0f;

    // Internal helpers
    void ApplyConsoleVarsForPreset(EPerf_TargetPlatform Platform, EPerf_QualityLevel Quality);
    float GetAverageFrameTime() const;
    void AutoAdjustQuality();
};
