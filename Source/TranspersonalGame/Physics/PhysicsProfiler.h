// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "HAL/ThreadSafeBool.h"
#include "Containers/CircularBuffer.h"
#include "PhysicsProfiler.generated.h"

USTRUCT(BlueprintType)
struct FPhysicsProfileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    float PhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float CollisionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float SimulationTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 ActiveConstraints = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 CollisionPairs = 0;

    UPROPERTY(BlueprintReadOnly)
    float MemoryUsageMB = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    float Timestamp = 0.0f;
};

USTRUCT(BlueprintType)
struct FPhysicsBottleneck
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString ComponentName;

    UPROPERTY(BlueprintReadOnly)
    FString ActorName;

    UPROPERTY(BlueprintReadOnly)
    float ProcessingTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    int32 CollisionCount = 0;

    UPROPERTY(BlueprintReadOnly)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FString BottleneckType;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPhysicsProfiler();

    // Profiling control
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartProfiling(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void UpdateProfiling(float DeltaTime);

    // Data access
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FPhysicsProfileData GetCurrentProfileData() const { return CurrentProfileData; }

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    TArray<FPhysicsProfileData> GetProfileHistory() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    TArray<FPhysicsBottleneck> GetCurrentBottlenecks() const { return CurrentBottlenecks; }

    // Analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    float GetAveragePhysicsTime(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    float GetPeakPhysicsTime(int32 SampleCount = 60) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool IsPhysicsBottlenecked(float ThresholdMs = 5.0f) const;

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FString GeneratePerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void ExportProfileDataToCSV(const FString& FilePath) const;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableDetailedProfiling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float ProfilingFrequency = 0.016667f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxHistorySize = 300; // 5 seconds at 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bTrackBottlenecks = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BottleneckThresholdMs = 2.0f;

protected:
    // Internal state
    UPROPERTY()
    TWeakObjectPtr<UWorld> WorldContext;

    UPROPERTY()
    FPhysicsProfileData CurrentProfileData;

    UPROPERTY()
    TArray<FPhysicsBottleneck> CurrentBottlenecks;

    UPROPERTY()
    bool bIsProfiling = false;

    UPROPERTY()
    float LastProfilingTime = 0.0f;

    // Performance history
    TCircularBuffer<FPhysicsProfileData> ProfileHistory;

    // Threading
    FThreadSafeBool bProfilingActive;

private:
    // Internal profiling methods
    void CollectPhysicsStats();
    void AnalyzeBottlenecks();
    void UpdateMemoryUsage();
    float MeasurePhysicsTime();
    float MeasureCollisionTime();
    float MeasureSimulationTime();
    void IdentifyExpensiveComponents();
    void TrackRigidBodyPerformance();
    void TrackConstraintPerformance();
    void CleanupOldData();

    // Timing helpers
    double PhysicsStartTime = 0.0;
    double CollisionStartTime = 0.0;
    double SimulationStartTime = 0.0;
};