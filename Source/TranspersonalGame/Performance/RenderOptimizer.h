// RenderOptimizer.h
// Sistema de otimização de rendering dinâmico

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialParameterCollection.h"
#include "RenderOptimizer.generated.h"

class UPerformanceProfiler;

// Estrutura para configurações de LOD dinâmico
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDynamicLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float DistanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float PerformanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAggressiveCulling = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistanceScale = 1.0f;

    FDynamicLODSettings()
        : DistanceMultiplier(1.0f)
        , PerformanceMultiplier(1.0f)
        , bEnableAggressiveCulling(false)
        , CullingDistanceScale(1.0f)
    {}
};

// Configurações de qualidade de sombras
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FShadowQualitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 ShadowMapResolution = 2048;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    int32 CascadeCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bEnableContactShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ContactShadowLength = 0.1f;

    FShadowQualitySettings()
        : ShadowMapResolution(2048)
        , ShadowDistance(5000.0f)
        , CascadeCount(4)
        , bEnableContactShadows(true)
        , ContactShadowLength(0.1f)
    {}
};

// Configurações de pós-processamento
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPostProcessSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    bool bEnableBloom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    bool bEnableSSAO = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    bool bEnableSSR = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float SSRQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    bool bEnableMotionBlur = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float MotionBlurAmount = 1.0f;

    FPostProcessSettings()
        : bEnableBloom(true)
        , bEnableSSAO(true)
        , bEnableSSR(true)
        , SSRQuality(1.0f)
        , bEnableMotionBlur(true)
        , MotionBlurAmount(1.0f)
    {}
};

// Enum para presets de qualidade
UENUM(BlueprintType)
enum class ERenderQualityPreset : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Custom      UMETA(DisplayName = "Custom")
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    URenderOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração de qualidade
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetRenderQualityPreset(ERenderQualityPreset Preset);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    ERenderQualityPreset GetCurrentQualityPreset() const { return CurrentQualityPreset; }

    // Otimização dinâmica
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void EnableDynamicOptimization(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    bool IsDynamicOptimizationEnabled() const { return bDynamicOptimizationEnabled; }

    // Configurações específicas
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetLODSettings(const FDynamicLODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetShadowQuality(const FShadowQualitySettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetPostProcessSettings(const FPostProcessSettings& Settings);

    // Culling e oclusão
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetFrustumCullingDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetOcclusionCullingEnabled(bool bEnabled);

    // Texture streaming
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void SetTextureStreamingPool(int32 PoolSizeMB);

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    void ForceTextureStreaming();

    // Monitoring
    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    int32 GetCurrentDrawCalls() const;

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    int32 GetCurrentTriangles() const;

    UFUNCTION(BlueprintCallable, Category = "Render Optimization")
    float GetCurrentGPUTime() const;

protected:
    // Configurações atuais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality Settings")
    ERenderQualityPreset CurrentQualityPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic LOD")
    FDynamicLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    FShadowQualitySettings ShadowSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    FPostProcessSettings PostProcessSettings;

    // Otimização dinâmica
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Optimization")
    bool bDynamicOptimizationEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Optimization")
    float OptimizationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Optimization")
    float TargetFrameTime;

    // Referências
    UPROPERTY()
    UPerformanceProfiler* PerformanceProfiler;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    UMaterialParameterCollection* GlobalRenderParams;

private:
    // Estado interno
    float LastOptimizationTime;
    bool bInitialized;
    
    // Cache de configurações
    TMap<ERenderQualityPreset, FDynamicLODSettings> LODPresets;
    TMap<ERenderQualityPreset, FShadowQualitySettings> ShadowPresets;
    TMap<ERenderQualityPreset, FPostProcessSettings> PostProcessPresets;

    // Métodos internos
    void InitializePresets();
    void ApplyQualityPreset(ERenderQualityPreset Preset);
    void UpdateDynamicOptimization(float DeltaTime);
    void AdjustLODBasedOnPerformance();
    void AdjustShadowsBasedOnPerformance();
    void AdjustPostProcessBasedOnPerformance();
    void UpdateGlobalRenderParameters();
    
    // Console variables cache
    void CacheConsoleVariables();
    void ApplyConsoleVariables();
    
    struct FConsoleVariableCache
    {
        int32 ShadowQuality;
        int32 PostProcessQuality;
        int32 EffectsQuality;
        int32 TextureQuality;
        float LODBias;
        float ViewDistanceScale;
    } CVarCache;
};