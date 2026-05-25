#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error"),
    NotTested   UMETA(DisplayName = "Not Tested")
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
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::NotTested;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FString> Issues;

    FQA_SystemHealth()
    {
        SystemName = TEXT("");
        bIsOperational = false;
        ActorCount = 0;
    }
};

/**
 * QA Validation Framework - Comprehensive testing system for TranspersonalGame
 * Validates all game systems, actors, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateDinosaurSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePerformanceMetrics();

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_SystemHealth> GetSystemHealthReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetOverallQAScore();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemReadyForIntegration();

    // Test result management
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_TestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ExportTestResultsToLog();

protected:
    // Internal validation helpers
    FQA_TestResult ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
    FQA_TestResult ValidateActorSpawning(UClass* ActorClass, const FVector& Location);
    FQA_TestResult ValidateComponentAttachment(AActor* Actor, UClass* ComponentClass);
    FQA_TestResult ValidateBiomeDistribution();
    FQA_TestResult ValidateActorPerformance();

    // Test result storage
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_SystemHealth> SystemHealthData;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float PerformanceThresholdFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCountThreshold;

private:
    void AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_TestResult& Result);
    float CalculateQAScore();
};