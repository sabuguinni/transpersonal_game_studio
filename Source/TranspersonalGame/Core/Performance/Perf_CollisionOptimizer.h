#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_CollisionOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_CollisionLOD : uint8
{
    Full        UMETA(DisplayName = "Full Collision"),
    Simplified  UMETA(DisplayName = "Simplified Collision"),
    Bounds      UMETA(DisplayName = "Bounds Only"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPerf_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Performance")
    FString ProfileName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Performance")
    float MaxTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Performance")
    int32 MaxSimultaneousTraces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Performance")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Performance")
    EPerf_CollisionLOD CollisionLOD;

    FPerf_CollisionProfile()
    {
        ProfileName = TEXT("Default");
        MaxTraceDistance = 10000.0f;
        MaxSimultaneousTraces = 100;
        LODDistance = 5000.0f;
        CollisionLOD = EPerf_CollisionLOD::Full;
    }
};

USTRUCT(BlueprintType)
struct FPerf_CollisionStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 ActiveTraces;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float AverageTraceTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    int32 TracesPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    float CollisionBudgetUsed;

    FPerf_CollisionStats()
    {
        ActiveTraces = 0;
        AverageTraceTime = 0.0f;
        TracesPerFrame = 0;
        CollisionBudgetUsed = 0.0f;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CollisionOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CollisionOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float CollisionBudgetMS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    int32 MaxTracesPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    float AdaptiveLODThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    bool bEnableAdaptiveLOD;

    // Collision profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Profiles")
    TArray<FPerf_CollisionProfile> CollisionProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Profiles")
    FString ActiveProfileName;

    // Performance monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    FPerf_CollisionStats CurrentStats;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Stats")
    TArray<float> FrameTimeHistory;

    // Optimization methods
    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void SetCollisionProfile(const FString& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    FPerf_CollisionProfile GetActiveProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void OptimizeCollisionForActor(AActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    bool ShouldPerformTrace(const FVector& Start, const FVector& End);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void RegisterTrace(float TraceTime);

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void UpdatePerformanceStats();

    UFUNCTION(BlueprintCallable, Category = "Collision Optimization")
    void AdaptLODBasedOnPerformance();

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawCollisionDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetPerformanceReport() const;

    UFUNCTION(CallInEditor, Category = "Debug")
    void RunCollisionBenchmark();

private:
    // Internal tracking
    TArray<float> TraceTimings;
    int32 CurrentFrameTraces;
    float LastFrameTime;
    float CollisionBudgetUsed;

    // Helper methods
    void InitializeDefaultProfiles();
    FPerf_CollisionProfile* FindProfile(const FString& ProfileName);
    void UpdateFrameStats(float DeltaTime);
    EPerf_CollisionLOD CalculateOptimalLOD(float Distance, float CurrentFrameTime);
};