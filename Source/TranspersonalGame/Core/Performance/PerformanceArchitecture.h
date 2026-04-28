#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "RenderingThread.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "PerformanceArchitecture.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPerformanceArchitecture, Log, All);

/**
 * Performance monitoring and optimization system
 * Ensures 60fps on PC and 30fps on console as per technical requirements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveNaniteInstances = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LumenReflections = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VirtualShadowMapPages = 0;
};

/**
 * Performance targets for different platforms
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceTargets
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxFrameTime = 16.67f; // 60fps = 16.67ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGPUTime = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxGameThreadTime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCalls = 3000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxTriangles = 10000000; // 10M triangles with Nanite

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB = 8192.0f; // 8GB

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxNaniteInstances = 1000000; // 1M Nanite instances
};

/**
 * LOD management for performance optimization
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_LODSettings_73E
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float NaniteLODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ShadowLODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float ParticleLODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FoliageLODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxShadowResolution = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxTextureResolution = 4096;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableNaniteHLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableVirtualShadowMaps = true;
};

/**
 * Performance Architecture Subsystem
 * Manages performance monitoring and automatic optimization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerformanceArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerformanceArchitecture();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceTargets(const FPerformanceTargets& NewTargets);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceTargets GetPerformanceTargets() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceTargetMet() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FCore_LODSettings_73E& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FCore_LODSettings_73E GetLODSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyPerformanceOptimizations();

    // Platform detection
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetupForPlatform();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsConsole() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPC() const;

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetMemoryUsageMB() const;

    // Nanite optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeNaniteSettings();

    // Lumen optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLumenSettings();

    // Virtual Shadow Maps optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVSMSettings();

protected:
    // Performance monitoring timer
    UPROPERTY()
    FTimerHandle PerformanceMonitorTimer;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PerformanceMetrics CurrentMetrics;

    // Performance targets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerformanceTargets PerformanceTargets;

    // LOD settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FCore_LODSettings_73E LODSettings;

    // Auto-optimization settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 FramesBelowTargetThreshold = 60; // 1 second at 60fps

    // Performance history
    UPROPERTY()
    TArray<float> FrameTimeHistory;

    UPROPERTY()
    int32 FramesBelowTarget = 0;

private:
    // Internal monitoring functions
    void UpdatePerformanceMetrics();
    void CheckPerformanceTargets();
    void AutoOptimize();
    
    // Platform-specific optimizations
    void ApplyConsoleOptimizations();
    void ApplyPCOptimizations();
    
    // Specific system optimizations
    void OptimizeForFrameRate(float TargetFrameTime);
    void ReduceRenderingLoad();
    void OptimizeMemoryUsage();
    
    // Console variable management
    void SetConsoleVariable(const FString& VariableName, float Value);
    void SetConsoleVariable(const FString& VariableName, int32 Value);
    void SetConsoleVariable(const FString& VariableName, bool bValue);
};

/**
 * Performance monitoring component for actors
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerformanceMonitorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerformanceMonitorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnablePerformanceMonitoring(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetActorRenderCost() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActorTriangleCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReduceLOD() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bMonitorPerformance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 16.67f; // 60fps target

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFrameRenderTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TriangleCount = 0;

private:
    void UpdateRenderCost();
    void CheckLODRequirements();
};