#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Perf_OptimizationManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_OptimizationLevel : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_OptimizationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxPhysicsActorRatio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutoLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_OptimizationLevel OptimizationLevel;

    FPerf_OptimizationSettings()
    {
        MaxPhysicsActorRatio = 15.0f;
        MaxActiveActors = 2000;
        CullingDistance = 10000.0f;
        bEnableAutoLOD = true;
        OptimizationLevel = EPerf_OptimizationLevel::Medium;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AverageFPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PhysicsActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsActorRatio;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DrawCallCount;

    FPerf_PerformanceMetrics()
    {
        CurrentFPS = 0.0f;
        AverageFPS = 0.0f;
        ActiveActorCount = 0;
        PhysicsActorCount = 0;
        PhysicsActorRatio = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCallCount = 0.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_OptimizationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_OptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Settings")
    FPerf_OptimizationSettings OptimizationSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    FPerf_PerformanceMetrics CurrentMetrics;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeScene();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetOptimizationLevel(EPerf_OptimizationLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsPerformanceAcceptable() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Performance", CallInEditor = true)
    void RunPerformanceAnalysis();

private:
    UPROPERTY()
    TArray<float> FPSHistory;

    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    int32 FrameCounter;

    void CalculateAverageFPS();
    void CheckPerformanceThresholds();
    void ApplyOptimizationSettings();
    TArray<AActor*> GetActorsInRadius(FVector Center, float Radius);
    void OptimizeActorLOD(AActor* Actor, float Distance);
};