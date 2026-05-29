#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalFramework.h"
#include "Eng_SystemValidator.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 005 - SYSTEM VALIDATOR
 * 
 * This subsystem provides comprehensive validation for all game systems.
 * It works in conjunction with the Architectural Framework to ensure:
 * 1. Runtime system integrity
 * 2. Performance compliance
 * 3. Memory usage validation
 * 4. Cross-system communication validation
 * 5. Build-time compilation safety
 */

// Validation Result Types
UENUM(BlueprintType)
enum class EEng_ValidationResult : uint8
{
    Pass            UMETA(DisplayName = "Pass"),
    Warning         UMETA(DisplayName = "Warning"),
    Error           UMETA(DisplayName = "Error"),
    Critical        UMETA(DisplayName = "Critical"),
    Unknown         UMETA(DisplayName = "Unknown")
};

// Validation Categories
UENUM(BlueprintType)
enum class EEng_ValidationCategory : uint8
{
    Performance     UMETA(DisplayName = "Performance"),
    Memory          UMETA(DisplayName = "Memory"),
    Compilation     UMETA(DisplayName = "Compilation"),
    Runtime         UMETA(DisplayName = "Runtime"),
    Integration     UMETA(DisplayName = "Integration"),
    Architecture    UMETA(DisplayName = "Architecture")
};

// Validation Test Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_ValidationCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EEng_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime TestTime;

    FEng_ValidationTest()
    {
        TestName = "Unknown Test";
        Category = EEng_ValidationCategory::Runtime;
        Result = EEng_ValidationResult::Unknown;
        ErrorMessage = "";
        ExecutionTime = 0.0f;
        TestTime = FDateTime::Now();
    }
};

// Validation Report
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FEng_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 CriticalCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FDateTime ReportTime;

    FEng_ValidationReport()
    {
        PassCount = 0;
        WarningCount = 0;
        ErrorCount = 0;
        CriticalCount = 0;
        TotalExecutionTime = 0.0f;
        ReportTime = FDateTime::Now();
    }
};

/**
 * SYSTEM VALIDATOR SUBSYSTEM
 * 
 * Provides comprehensive validation services for all game systems.
 * This is the quality assurance backbone of the architectural framework.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemValidator : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Validation Control
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool StartValidation();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool StopValidation();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool IsValidationRunning();

    // Test Execution
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationReport RunCategoryValidation(EEng_ValidationCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest RunSingleTest(const FString& TestName);

    // Performance Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidatePhysicsPerformance();

    // System Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateArchitecturalCompliance();

    // Compilation Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateTypeRegistry();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateHeaderIncludes();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateSharedTypes();

    // Integration Validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateCrossSystemCommunication();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationTest ValidateGameplayIntegration();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Validation")
    FEng_ValidationReport GetLastValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FEng_ValidationTest> GetFailedTests();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool HasCriticalErrors();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Validation")
    void SetValidationInterval(float Interval);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void EnableContinuousValidation(bool bEnable);

protected:
    // Validation state
    UPROPERTY()
    bool bValidationRunning;

    UPROPERTY()
    bool bContinuousValidation;

    UPROPERTY(EditAnywhere, Category = "Validation")
    float ValidationInterval;

    // Reports and results
    UPROPERTY()
    FEng_ValidationReport LastReport;

    UPROPERTY()
    TArray<FEng_ValidationTest> FailedTests;

    // Framework reference
    UPROPERTY()
    UEng_ArchitecturalFramework* ArchFramework;

    // Internal methods
    void RunContinuousValidation();
    FEng_ValidationTest CreateTest(const FString& TestName, EEng_ValidationCategory Category);
    void UpdateTestResult(FEng_ValidationTest& Test, EEng_ValidationResult Result, const FString& Message = "");
    void AddTestToReport(FEng_ValidationReport& Report, const FEng_ValidationTest& Test);
    void FinalizeReport(FEng_ValidationReport& Report);

private:
    // Validation timer
    FTimerHandle ValidationTimer;
    
    // Test execution tracking
    float CurrentTestStartTime;
};