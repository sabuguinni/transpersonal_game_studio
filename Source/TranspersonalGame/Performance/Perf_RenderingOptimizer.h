#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/SkeletalMeshActor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Perf_RenderingOptimizer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float NearDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MidDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FarDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 25000.0f;

    FPerf_LODSettings()
    {
        NearDistance = 1000.0f;
        MidDistance = 5000.0f;
        FarDistance = 15000.0f;
        CullDistance = 25000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_RenderStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 TotalMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 VisibleMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    int32 CulledMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float AverageDrawCalls = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    float MemoryUsageMB = 0.0f;

    FPerf_RenderStats()
    {
        TotalMeshes = 0;
        VisibleMeshes = 0;
        CulledMeshes = 0;
        AverageDrawCalls = 0.0f;
        MemoryUsageMB = 0.0f;
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

    // World Subsystem interface
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyDistanceCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeMaterialInstances();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateRenderingStats();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_RenderStats GetCurrentRenderStats() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableOcclusionCulling(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxDrawDistance(float Distance);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Stats")
    FPerf_RenderStats CurrentStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bOcclusionCullingEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxDrawDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateInterval = 1.0f;

private:
    FTimerHandle UpdateTimerHandle;

    void ProcessStaticMeshActors();
    void ProcessSkeletalMeshActors();
    void ApplyLODToMesh(UStaticMeshComponent* MeshComp, float Distance);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* SkeletalComp, float Distance);
    float CalculateDistanceToPlayer(const FVector& ActorLocation) const;
};