#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "QA_VFXValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotTested = 0,
    Pass = 1,
    Warning = 2,
    Fail = 3
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    EQA_VFXTestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    float ExecutionTime;

    FQA_VFXTestCase()
    {
        TestName = TEXT("Unknown Test");
        Result = EQA_VFXTestResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_VFXPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 ActiveParticleSystems;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "QA Performance")
    bool bPerformanceAcceptable;

    FQA_VFXPerformanceMetrics()
    {
        ActiveParticleSystems = 0;
        TotalActorsInLevel = 0;
        FrameTime = 0.0f;
        MemoryUsageMB = 0.0f;
        bPerformanceAcceptable = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXValidationFramework();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestNiagaraSystemLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXComponentAttachment();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestVFXPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXAudioIntegration();

    // Test execution and reporting
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunFullVFXValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateVFXValidationReport();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_VFXPerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsVFXPerformanceAcceptable();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test case management
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_VFXTestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FQA_VFXPerformanceMetrics CurrentMetrics;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    int32 MaxAcceptableActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    float MaxAcceptableMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Configuration")
    bool bAutoRunValidationOnBeginPlay;

private:
    // Internal validation helpers
    bool ValidateVFXManagerClasses();
    bool ValidateNiagaraAvailability();
    bool TestVFXActorSpawning();
    bool ValidateVFXIntegrationPoints();
    
    // Performance monitoring helpers
    void UpdatePerformanceMetrics();
    bool CheckFrameTimeThreshold();
    bool CheckMemoryUsageThreshold();
    bool CheckActorCountThreshold();

    // Test result management
    void AddTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void ClearTestResults();

    // Timing
    float TestStartTime;
    bool bValidationInProgress;
};