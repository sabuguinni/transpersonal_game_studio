#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Perf_PerformanceTestSuite.generated.h"

UENUM(BlueprintType)
enum class EPerf_TestType : uint8
{
    None = 0,
    FrameRate = 1,
    Memory = 2,
    Physics = 3,
    Rendering = 4,
    AI = 5,
    Streaming = 6,
    All = 7
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_TestType TestType;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float Score;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FString Details;

    FPerf_TestResult()
        : TestType(EPerf_TestType::None)
        , Score(0.0f)
        , bPassed(false)
        , Details(TEXT(""))
    {}
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PerformanceTestSuite : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PerformanceTestSuite();

    // Core test functions
    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunFrameRateTest(float Duration = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunMemoryTest();

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunPhysicsTest();

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunRenderingTest();

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunAITest();

    UFUNCTION(BlueprintCallable, Category = "Performance Testing")
    FPerf_TestResult RunStreamingTest();

    // Results access
    UFUNCTION(BlueprintPure, Category = "Performance Testing")
    TArray<FPerf_TestResult> GetAllTestResults() const { return TestResults; }

    UFUNCTION(BlueprintPure, Category = "Performance Testing")
    FPerf_TestResult GetTestResult(EPerf_TestType TestType) const;

    UFUNCTION(BlueprintPure, Category = "Performance Testing")
    bool AllTestsPassed() const;

    UFUNCTION(BlueprintPure, Category = "Performance Testing")
    float GetOverallScore() const;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing")
    float TargetFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing")
    int32 MaxPhysicsObjects;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Testing")
    bool bAutoRunOnBeginPlay;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY()
    TArray<FPerf_TestResult> TestResults;

    // Internal test helpers
    float MeasureAverageFrameRate(float Duration);
    float GetCurrentMemoryUsageMB();
    int32 CountPhysicsObjects();
    float MeasureRenderTime();
    float MeasureAIProcessingTime();
    float MeasureStreamingPerformance();

    void LogTestResult(const FPerf_TestResult& Result);
    FPerf_TestResult CreateTestResult(EPerf_TestType TestType, float Score, bool bPassed, const FString& Details);
};