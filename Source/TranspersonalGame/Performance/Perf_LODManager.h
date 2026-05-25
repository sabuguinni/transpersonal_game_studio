#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Perf_LODManager.generated.h"

/**
 * Advanced LOD (Level of Detail) Management System
 * Optimizes rendering performance by dynamically adjusting mesh detail based on distance
 * Implements biome-based culling zones and performance monitoring
 * Ensures 60fps on PC and 30fps on Console platforms
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Distance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 2000.0f;  // 20 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 5000.0f;  // 50 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 10000.0f;  // 100 meters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 20000.0f;  // 200 meters

    // Performance Targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_PC = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS_Console = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVisibleActors = 1000;

    // Biome Culling Zones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Culling")
    TArray<FBiomeZone> BiomeZones;

    // LOD Management Functions
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void OptimizeActorLOD(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetActorLODLevel(AActor* Actor, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void EnableDistanceCulling(AActor* Actor, float MaxDistance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void OptimizeDinosaurActors();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void InitializeBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    FBiomeZone GetNearestBiomeZone(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Zone Management")
    void OptimizeZoneActors(const FBiomeZone& Zone);

    // Memory Optimization
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RunGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void OptimizeStreamingPool();

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void GeneratePerformanceReport();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void SpawnPerformanceTestObjects();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidatePerformanceTargets();

private:
    // Internal tracking
    UPROPERTY()
    TArray<AActor*> ManagedActors;

    UPROPERTY()
    TArray<AActor*> DinosaurActors;

    UPROPERTY()
    float LastFrameTime;

    UPROPERTY()
    int32 CurrentVisibleActors;

    // Performance metrics
    UPROPERTY()
    float AverageFrameTime;

    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 MaxFrameHistorySize = 60;  // 1 second at 60fps

    // Internal functions
    void UpdateFrameTimeHistory(float DeltaTime);
    void UpdateVisibleActorCount();
    void ApplyLODOptimizations();
    void CheckPerformanceThresholds();
    int32 CalculateLODLevel(float Distance) const;
    void CullDistantActors();
    void OptimizePhysicsComponents();
    
    // Platform detection
    bool IsRunningOnConsole() const;
    float GetTargetFPS() const;
};