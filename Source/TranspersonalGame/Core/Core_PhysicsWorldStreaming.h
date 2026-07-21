#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/WorldComposition.h"
#include "Engine/LevelStreaming.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Core_PhysicsWorldStreaming.generated.h"

// Forward declarations
class UCore_PhysicsManager;
class UCore_PhysicsSimulationManager;

/**
 * Physics streaming level data
 * Tracks physics state for streaming levels
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsLevelData
{
    GENERATED_BODY()

    // Level name
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    FString LevelName;

    // Physics bodies count in this level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    int32 PhysicsBodiesCount;

    // Memory usage for physics in this level (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    float PhysicsMemoryUsageMB;

    // Is physics simulation active for this level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    bool bPhysicsActive;

    // Level streaming distance
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    float StreamingDistance;

    // Constructor
    FCore_PhysicsLevelData()
    {
        LevelName = TEXT("");
        PhysicsBodiesCount = 0;
        PhysicsMemoryUsageMB = 0.0f;
        bPhysicsActive = false;
        StreamingDistance = 10000.0f;
    }
};

/**
 * Physics streaming performance metrics
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsStreamingMetrics
{
    GENERATED_BODY()

    // Total loaded levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    int32 LoadedLevelsCount;

    // Total physics bodies across all levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    int32 TotalPhysicsBodies;

    // Total physics memory usage (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    float TotalPhysicsMemoryMB;

    // Streaming operations per second
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    float StreamingOpsPerSecond;

    // Average level load time (ms)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming")
    float AverageLevelLoadTimeMS;

    // Constructor
    FCore_PhysicsStreamingMetrics()
    {
        LoadedLevelsCount = 0;
        TotalPhysicsBodies = 0;
        TotalPhysicsMemoryMB = 0.0f;
        StreamingOpsPerSecond = 0.0f;
        AverageLevelLoadTimeMS = 0.0f;
    }
};

/**
 * Core Physics World Streaming System
 * Manages physics simulation across streaming levels for optimal performance
 * Integrates with UE5 World Partition and Level Streaming
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UCore_PhysicsWorldStreaming : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Constructor
    UCore_PhysicsWorldStreaming();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core streaming functions
    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void InitializePhysicsStreaming();

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void UpdatePhysicsStreaming(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void ShutdownPhysicsStreaming();

    // Level management
    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void RegisterStreamingLevel(const FString& LevelName, float StreamingDistance);

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void UnregisterStreamingLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void SetLevelPhysicsActive(const FString& LevelName, bool bActive);

    // Physics optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void OptimizePhysicsForLevel(const FString& LevelName);

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void SetPhysicsLODForLevel(const FString& LevelName, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming")
    void UpdatePhysicsMemoryBudget(float MaxMemoryMB);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Streaming")
    FCore_PhysicsStreamingMetrics GetStreamingMetrics() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Streaming")
    TArray<FCore_PhysicsLevelData> GetActiveLevelsData() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics Streaming")
    float GetPhysicsMemoryUsage() const;

    // Debug functions
    UFUNCTION(BlueprintCallable, Category = "Physics Streaming", CallInEditor)
    void DebugPrintStreamingStatus();

    UFUNCTION(BlueprintCallable, Category = "Physics Streaming", CallInEditor)
    void DebugVisualizeStreamingBounds();

protected:
    // Streaming level data
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsLevelData> StreamingLevels;

    // Current streaming metrics
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsStreamingMetrics CurrentMetrics;

    // Physics memory budget (MB)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    float PhysicsMemoryBudgetMB;

    // Streaming update frequency (Hz)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    float StreamingUpdateFrequency;

    // Auto-optimize physics for streaming levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    bool bAutoOptimizePhysics;

    // Enable physics LOD for distant levels
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Streaming", meta = (AllowPrivateAccess = "true"))
    bool bEnablePhysicsLOD;

private:
    // Internal update timer
    float StreamingUpdateTimer;

    // Physics managers
    UPROPERTY()
    UCore_PhysicsManager* PhysicsManager;

    UPROPERTY()
    UCore_PhysicsSimulationManager* SimulationManager;

    // Internal functions
    void UpdateLevelPhysicsData(const FString& LevelName);
    void CheckMemoryBudget();
    void OptimizePhysicsLOD();
    void HandleLevelLoaded(ULevel* LoadedLevel);
    void HandleLevelUnloaded(ULevel* UnloadedLevel);
    float CalculateLevelPhysicsMemory(ULevel* Level) const;
    int32 CountPhysicsBodiesInLevel(ULevel* Level) const;
};