#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }

    FQA_TestResult(const FString& InTestName, EQA_ValidationResult InResult, const FString& InMessage, float InExecutionTime)
        : TestName(InTestName), Result(InResult), Message(InMessage), ExecutionTime(InExecutionTime)
    {
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 InstanceCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> ErrorMessages;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        InstanceCount = 0;
    }
};

/**
 * QA Validation Framework - Comprehensive testing system for TranspersonalGame
 * Provides automated testing, performance monitoring, and system validation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE VALIDATION FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA|Validation")
    void ValidateAssetPipeline();

    // === PERFORMANCE TESTING ===

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    void RunPerformanceStressTest();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    void MonitorFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA|Performance")
    void CheckMemoryUsage();

    // === SYSTEM STATUS ===

    UFUNCTION(BlueprintCallable, Category = "QA|Status")
    TArray<FQA_SystemStatus> GetSystemStatusReport();

    UFUNCTION(BlueprintCallable, Category = "QA|Status")
    int32 GetTotalActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA|Status")
    TArray<FString> GetCustomClassInstances() const;

    // === RESULTS AND REPORTING ===

    UFUNCTION(BlueprintCallable, Category = "QA|Results")
    TArray<FQA_TestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA|Results")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA|Results")
    void ExportResultsToLog();

    UFUNCTION(BlueprintCallable, Category = "QA|Results")
    FString GenerateValidationReport();

protected:
    // === INTERNAL TESTING FUNCTIONS ===

    FQA_TestResult RunSingleTest(const FString& TestName, TFunction<bool()> TestFunction);
    
    void AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);

    bool TestClassLoading(const FString& ClassName);
    
    bool TestActorSpawning(UClass* ActorClass, const FVector& Location);
    
    bool TestComponentFunctionality(AActor* Actor, const FString& ComponentName);

    // === PROPERTIES ===

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    bool bAutoRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    bool bEnablePerformanceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Settings")
    int32 MaxTestActorsToSpawn;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Status")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "QA|Status")
    bool bValidationInProgress;

private:
    // === INTERNAL STATE ===
    
    float TimeSinceLastValidation;
    
    TArray<AActor*> SpawnedTestActors;
    
    // Critical system class names to validate
    TArray<FString> CriticalSystemClasses;
    
    void InitializeCriticalSystems();
    void CleanupTestActors();
};