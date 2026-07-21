#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_RenderingOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High = 0    UMETA(DisplayName = "High Quality"),
    Medium = 1  UMETA(DisplayName = "Medium Quality"),
    Low = 2     UMETA(DisplayName = "Low Quality"),
    Minimal = 3 UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderingStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 VisibleActors;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 ShadowCasters;

    UPROPERTY(BlueprintReadOnly, Category = "Rendering")
    int32 LightCount;

    FPerf_RenderingStats()
    {
        DrawCalls = 0;
        Triangles = 0;
        VisibleActors = 0;
        GPUTime = 0.0f;
        RenderThreadTime = 0.0f;
        ShadowCasters = 0;
        LightCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowQualityDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance;

    FPerf_LODSettings()
    {
        HighQualityDistance = 1000.0f;
        MediumQualityDistance = 3000.0f;
        LowQualityDistance = 8000.0f;
        CullingDistance = 15000.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UPerf_RenderingOptimizer : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_RenderingOptimizer();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetGlobalLODLevel(EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_RenderingStats GetRenderingStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeShadowCasting();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantObjects();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableOcclusionCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetTextureQuality(int32 Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForFrameRate(float TargetFPS);

protected:
    UPROPERTY()
    FPerf_LODSettings CurrentLODSettings;

    UPROPERTY()
    EPerf_LODLevel CurrentGlobalLOD;

    UPROPERTY()
    bool bAutoOptimizationEnabled;

    UPROPERTY()
    float TargetFrameRate;

    FTimerHandle OptimizationTimerHandle;

    void AutoOptimizeRendering();
    void OptimizeMeshLOD(UStaticMeshComponent* MeshComponent, float Distance);
    void OptimizeSkeletalMeshLOD(USkeletalMeshComponent* SkeletalMesh, float Distance);
    float GetDistanceToPlayer(const FVector& Location);
    void UpdateShadowSettings(EPerf_LODLevel LODLevel);
    void UpdateLightingSettings(EPerf_LODLevel LODLevel);
    int32 CountVisibleActors();
    void LogRenderingStats();
};