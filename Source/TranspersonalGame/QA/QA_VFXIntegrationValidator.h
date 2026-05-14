#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "QA_VFXIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotTested = 0,
    Pass = 1,
    Fail = 2,
    Warning = 3
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float ExecutionTime;

    FQA_VFXTestCase()
    {
        TestName = TEXT("Unnamed Test");
        Result = EQA_VFXTestResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 VFXActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ParticleSystemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        TotalActorCount = 0;
        VFXActorCount = 0;
        ParticleSystemCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * QA VFX Integration Validator - Comprehensive testing system for VFX integration
 * Validates particle systems, performance impact, and integration with game systems
 */
UCLASS(ClassGroup=(QA), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_VFXIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXSystemCompilationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXSpawningTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXIntegrationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXCleanupTest();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    // Test result management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_VFXTestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool AreAllTestsPassing() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FString GenerateTestReport() const;

    // VFX system validation
    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool ValidateVFXManagerClass();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool ValidateParticleSystemAssets();

    UFUNCTION(BlueprintCallable, Category = "VFX Validation")
    bool ValidateVFXComponentIntegration();

protected:
    // Test data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_VFXTestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bContinuousPerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float PerformanceMonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxAllowedVFXActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MinRequiredFrameRate;

    // Internal state
    UPROPERTY()
    bool bIsMonitoringPerformance;

    UPROPERTY()
    float LastPerformanceCheck;

    UPROPERTY()
    TArray<AActor*> TestActors;

private:
    // Helper functions
    void AddTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_VFXTestCase& TestCase);
    void UpdatePerformanceMetrics();
    void CleanupTestActors();
    
    // VFX specific validation helpers
    bool CheckVFXClassAvailability();
    bool CheckParticleSystemSpawning();
    bool CheckVFXComponentAttachment();
    bool CheckVFXSystemCleanup();
};