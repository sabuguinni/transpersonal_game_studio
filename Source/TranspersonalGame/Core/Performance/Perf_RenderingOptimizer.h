#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "RHI.h"
#include "RenderingThread.h"
#include "Perf_RenderingOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RenderQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"), 
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    Custom      UMETA(DisplayName = "Custom Settings")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    float DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    float Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    float GPUMemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering Stats")
    int32 VisibleActors;

    FPerf_RenderingStats()
    {
        DrawCalls = 0.0f;
        Triangles = 0.0f;
        GPUMemoryUsageMB = 0.0f;
        RenderThreadTime = 0.0f;
        GameThreadTime = 0.0f;
        VisibleActors = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 ShadowQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 TextureQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 EffectsQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    int32 PostProcessQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    float ViewDistanceScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    float FoliageDensityScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    bool bEnableLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    bool bEnableNanite;

    FPerf_RenderingSettings()
    {
        ShadowQuality = 3;
        TextureQuality = 3;
        EffectsQuality = 3;
        PostProcessQuality = 3;
        ViewDistanceScale = 1.0f;
        FoliageDensityScale = 1.0f;
        bEnableLumen = true;
        bEnableNanite = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPerf_OnRenderingOptimized, EPerf_RenderQuality, NewQuality);

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
    // Core optimization functions
    UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
    void OptimizeForTargetFPS(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
    void SetRenderQuality(EPerf_RenderQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
    void ApplyCustomSettings(const FPerf_RenderingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Rendering Optimization")
    void ResetToDefaultSettings();

    // Monitoring functions
    UFUNCTION(BlueprintCallable, Category = "Rendering Monitoring")
    FPerf_RenderingStats GetCurrentRenderingStats() const;

    UFUNCTION(BlueprintCallable, Category = "Rendering Monitoring")
    float GetCurrentGPUUsage() const;

    UFUNCTION(BlueprintCallable, Category = "Rendering Monitoring")
    int32 GetCurrentDrawCalls() const;

    // LOD optimization
    UFUNCTION(BlueprintCallable, Category = "LOD Optimization")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "LOD Optimization")
    void SetGlobalLODBias(float LODBias);

    // Shadow optimization
    UFUNCTION(BlueprintCallable, Category = "Shadow Optimization")
    void OptimizeShadowSettings();

    UFUNCTION(BlueprintCallable, Category = "Shadow Optimization")
    void SetShadowDistance(float Distance);

    // Texture optimization
    UFUNCTION(BlueprintCallable, Category = "Texture Optimization")
    void OptimizeTextureSettings();

    UFUNCTION(BlueprintCallable, Category = "Texture Optimization")
    void SetTextureStreamingPoolSize(int32 SizeMB);

    // Post-process optimization
    UFUNCTION(BlueprintCallable, Category = "Post Process Optimization")
    void OptimizePostProcessSettings();

    // Culling optimization
    UFUNCTION(BlueprintCallable, Category = "Culling Optimization")
    void OptimizeCullingSettings();

    UFUNCTION(BlueprintCallable, Category = "Culling Optimization")
    void SetViewDistanceScale(float Scale);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FPerf_OnRenderingOptimized OnRenderingOptimized;

protected:
    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float OptimizationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EPerf_RenderQuality CurrentQuality;

    // Current settings
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Settings")
    FPerf_RenderingSettings CurrentSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current Settings")
    FPerf_RenderingSettings DefaultSettings;

    // Statistics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    FPerf_RenderingStats CurrentStats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    float LastOptimizationTime;

private:
    // Internal functions
    void UpdateRenderingStats();
    void ApplyQualityPreset(EPerf_RenderQuality Quality);
    void ExecuteConsoleCommand(const FString& Command);
    void CheckAutoOptimization();
    
    // Quality presets
    FPerf_RenderingSettings GetLowQualitySettings() const;
    FPerf_RenderingSettings GetMediumQualitySettings() const;
    FPerf_RenderingSettings GetHighQualitySettings() const;
    FPerf_RenderingSettings GetUltraQualitySettings() const;

    // Timer handle for optimization checks
    FTimerHandle OptimizationTimerHandle;
};

#include "Perf_RenderingOptimizer.generated.h"