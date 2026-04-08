/**
 * @file RenderOptimizer.h
 * @brief Rendering optimization system for consciousness-based visual effects
 * 
 * Manages LOD, culling, and quality settings for spiritual visual effects,
 * consciousness fields, and transcendental experiences.
 */

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Materials/MaterialParameterCollection.h"
#include "RenderOptimizer.generated.h"

UENUM(BlueprintType)
enum class ESpiritualEffectQuality : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct FRenderPerformanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ESpiritualEffectQuality EffectQuality = ESpiritualEffectQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessFieldResolution = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxSpiritualParticles = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AuraRenderDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableConsciousnessBloom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableSpiritualSSR = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualLightingComplexity = 1.0f;
};

USTRUCT(BlueprintType)
struct FRenderStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 SpiritualEffectDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 VisibleConsciousnessActors = 0;

    UPROPERTY(BlueprintReadOnly)
    float VRAMUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveSpiritualParticles = 0;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    URenderOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, 
                              FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Quality management
    UFUNCTION(BlueprintCallable, Category = "Render Quality")
    void SetSpiritualEffectQuality(ESpiritualEffectQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Render Quality")
    ESpiritualEffectQuality GetCurrentQuality() const { return CurrentSettings.EffectQuality; }

    UFUNCTION(BlueprintCallable, Category = "Render Quality")
    void SetRenderSettings(const FRenderPerformanceSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Render Quality")
    FRenderPerformanceSettings GetRenderSettings() const { return CurrentSettings; }

    // Adaptive rendering
    UFUNCTION(BlueprintCallable, Category = "Adaptive Rendering")
    void EnableAdaptiveRendering(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Rendering")
    void SetTargetGPUTime(float TargetMS);

    // Consciousness field optimization
    UFUNCTION(BlueprintCallable, Category = "Consciousness Fields")
    void OptimizeConsciousnessFieldRendering();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Fields")
    void SetConsciousnessFieldLOD(float LODLevel);

    // Spiritual particle optimization
    UFUNCTION(BlueprintCallable, Category = "Spiritual Particles")
    void OptimizeSpiritualParticles();

    UFUNCTION(BlueprintCallable, Category = "Spiritual Particles")
    void SetMaxSpiritualParticles(int32 MaxParticles);

    // Aura and energy rendering
    UFUNCTION(BlueprintCallable, Category = "Aura Rendering")
    void OptimizeAuraRendering();

    UFUNCTION(BlueprintCallable, Category = "Aura Rendering")
    void SetAuraRenderDistance(float Distance);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FRenderStats GetRenderStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogRenderPerformance() const;

    // Material optimization
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void OptimizeSpiritualMaterials();

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateMaterialParameterCollection();

protected:
    // Current settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FRenderPerformanceSettings CurrentSettings;

    // Adaptive rendering configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    bool bAdaptiveRenderingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float TargetGPUTimeMS = 16.67f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float AdaptiveCheckInterval = 0.5f;

    // Material parameter collection for global spiritual effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialParameterCollection> SpiritualParameterCollection;

    // Render targets for consciousness effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Targets")
    TSoftObjectPtr<UTextureRenderTarget2D> ConsciousnessFieldRT;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Targets")
    TSoftObjectPtr<UTextureRenderTarget2D> AuraCompositeRT;

private:
    FRenderStats CurrentStats;
    float AdaptiveTimer = 0.0f;
    TArray<float> GPUTimeHistory;
    static constexpr int32 GPUTimeHistorySize = 30;

    // Internal optimization methods
    void UpdateRenderStats();
    void CheckAdaptiveRendering();
    void ApplyQualitySettings(ESpiritualEffectQuality Quality);
    void UpdateConsciousnessFieldParameters();
    void CullDistantSpiritualEffects();
    void OptimizeShaderComplexity();
    ESpiritualEffectQuality CalculateOptimalQuality() const;
};