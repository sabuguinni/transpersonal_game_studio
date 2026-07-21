#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeOptimizationManager.generated.h"

USTRUCT(BlueprintType)
struct FWorld_BiomeOptimizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxLODLevel = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float DetailCullingDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxInstanceCount = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PerformanceThreshold = 30.0f;

    FWorld_BiomeOptimizationData()
    {
        BiomeType = EBiomeType::Forest;
        MaxLODLevel = 4;
        CullingDistance = 5000.0f;
        DetailCullingDistance = 2000.0f;
        MaxInstanceCount = 1000;
        PerformanceThreshold = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_LODTransitionZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float TransitionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 FromLODLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 ToLODLevel = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float TransitionSpeed = 2.0f;

    FWorld_LODTransitionZone()
    {
        ZoneCenter = FVector::ZeroVector;
        TransitionRadius = 500.0f;
        FromLODLevel = 0;
        ToLODLevel = 1;
        TransitionSpeed = 2.0f;
    }
};

/**
 * Manages biome performance optimization including LOD transitions, culling, and instance management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeOptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeOptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USphereComponent* OptimizationZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VisualizationMesh;

    // Optimization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    FWorld_BiomeOptimizationData OptimizationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    TArray<FWorld_LODTransitionZone> TransitionZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableAdaptiveLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableInstancePooling = true;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CurrentInstanceCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CullingEfficiency = 1.0f;

private:
    // Internal state
    float LastPerformanceCheck = 0.0f;
    TArray<class AActor*> ManagedActors;
    TMap<int32, TArray<class UStaticMeshComponent*>> LODLevelInstances;

public:
    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void InitializeOptimization();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void PerformCullingPass();

    UFUNCTION(BlueprintCallable, Category = "Optimization")
    void OptimizeInstanceCounts();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CheckPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceScore() const;

    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "LOD")
    void SetBiomeLODLevel(int32 NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD")
    int32 CalculateOptimalLODLevel() const;

    UFUNCTION(BlueprintCallable, Category = "LOD")
    void CreateLODTransitionZone(const FWorld_LODTransitionZone& TransitionZone);

    // Culling Functions
    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableDistanceCulling(float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableFrustumCulling();

    UFUNCTION(BlueprintCallable, Category = "Culling")
    void EnableOcclusionCulling();

    // Instance Management
    UFUNCTION(BlueprintCallable, Category = "Instances")
    void RegisterManagedActor(class AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Instances")
    void UnregisterManagedActor(class AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Instances")
    void PoolUnusedInstances();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetOptimizationRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    void SetBiomeType(EBiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Utility", CallInEditor)
    void DebugOptimizationState();

    UFUNCTION(BlueprintCallable, Category = "Utility", CallInEditor)
    void ResetOptimization();
};