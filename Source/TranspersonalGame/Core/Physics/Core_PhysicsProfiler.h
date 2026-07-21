#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Core_PhysicsProfiler.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProfileData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 ActiveRigidBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 PhysicsConstraints;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    int32 CollisionQueries;

    FCore_PhysicsProfileData()
        : FrameTime(0.0f)
        , ActiveRigidBodies(0)
        , PhysicsConstraints(0)
        , PhysicsStepTime(0.0f)
        , CollisionQueries(0)
    {
    }
};

/**
 * Physics profiler component for monitoring physics performance
 * Tracks physics simulation metrics and provides optimization data
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsProfiler : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsProfiler();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Start profiling physics performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StartProfiling();

    /** Stop profiling physics performance */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void StopProfiling();

    /** Get current physics profile data */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PhysicsProfileData GetCurrentProfileData() const;

    /** Get average physics profile data over time */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    FCore_PhysicsProfileData GetAverageProfileData() const;

    /** Reset profiling data */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    void ResetProfilingData();

    /** Check if physics performance is within acceptable limits */
    UFUNCTION(BlueprintCallable, Category = "Physics Profiling")
    bool IsPhysicsPerformanceAcceptable() const;

protected:
    /** Current profiling state */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    bool bIsProfiling;

    /** Current frame profile data */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Profiling")
    FCore_PhysicsProfileData CurrentFrameData;

    /** Accumulated profile data for averaging */
    UPROPERTY()
    TArray<FCore_PhysicsProfileData> ProfileDataHistory;

    /** Maximum number of frames to keep in history */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Profiling", meta = (ClampMin = "10", ClampMax = "1000"))
    int32 MaxHistoryFrames;

    /** Target frame time threshold (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Profiling", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float TargetFrameTime;

    /** Maximum acceptable physics step time (ms) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Profiling", meta = (ClampMin = "1.0", ClampMax = "50.0"))
    float MaxPhysicsStepTime;

private:
    /** Update current frame physics data */
    void UpdateCurrentFrameData();

    /** Add current frame data to history */
    void AddFrameDataToHistory();

    /** Calculate physics metrics */
    void CalculatePhysicsMetrics();

    /** Last frame time for delta calculations */
    float LastFrameTime;

    /** Physics world reference */
    UPROPERTY()
    UWorld* PhysicsWorld;
};