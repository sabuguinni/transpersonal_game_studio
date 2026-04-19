#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "UObject/NoExportTypes.h"
#include "../SharedTypes.h"
#include "BuildValidationSystem.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    NotTested = 0,
    Passed = 1,
    Failed = 2,
    Warning = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ClassValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ClassName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bClassLoadable;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCDOAccessible;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bActorSpawnable;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_ValidationResult ValidationResult;

    FBuild_ClassValidationData()
    {
        ClassName = TEXT("");
        bClassLoadable = false;
        bCDOAccessible = false;
        bActorSpawnable = false;
        ErrorMessage = TEXT("");
        ValidationResult = EBuild_ValidationResult::NotTested;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    TArray<FBuild_ClassValidationData> ClassValidations;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 WarningTests;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ValidationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FDateTime LastValidationTime;

    FBuild_ModuleValidationReport()
    {
        ModuleName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        WarningTests = 0;
        ValidationScore = 0.0f;
        LastValidationTime = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString TestDescription;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bTestPassed;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float ExecutionTimeMs;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString ResultMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> DependentSystems;

    FBuild_IntegrationTestResult()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        bTestPassed = false;
        ExecutionTimeMs = 0.0f;
        ResultMessage = TEXT("");
    }
};

/**
 * Build Validation System - Comprehensive validation and integration testing
 * Validates compilation, class loading, actor spawning, and cross-system integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildValidationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuildValidationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ModuleValidationReport ValidateModule(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_ClassValidationData ValidateClass(const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    TArray<FBuild_IntegrationTestResult> RunIntegrationTests();

    // Specific validation tests
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateClassLoading(const FString& ClassPath, FString& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateCDOAccess(UClass* TestClass, FString& OutErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    bool ValidateActorSpawning(UClass* ActorClass, FString& OutErrorMessage);

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_IntegrationTestResult TestWorldGenerationIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_IntegrationTestResult TestCharacterSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_IntegrationTestResult TestAISystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FBuild_IntegrationTestResult TestCombatSystemIntegration();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    FString GetValidationSummary() const;

    UFUNCTION(BlueprintCallable, Category = "Build Validation")
    float GetOverallValidationScore() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Build Validation")
    const TArray<FBuild_ModuleValidationReport>& GetModuleReports() const { return ModuleReports; }

    UFUNCTION(BlueprintPure, Category = "Build Validation")
    const TArray<FBuild_IntegrationTestResult>& GetIntegrationResults() const { return IntegrationResults; }

    UFUNCTION(BlueprintPure, Category = "Build Validation")
    bool IsValidationComplete() const { return bValidationComplete; }

protected:
    // Core validation data
    UPROPERTY(BlueprintReadOnly, Category = "Validation Data")
    TArray<FBuild_ModuleValidationReport> ModuleReports;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Data")
    TArray<FBuild_IntegrationTestResult> IntegrationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Data")
    bool bValidationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Data")
    FDateTime LastFullValidation;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Data")
    float OverallValidationScore;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    TArray<FString> CoreClassPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    TArray<FString> ModulesToValidate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bRunIntegrationTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Config")
    bool bVerboseLogging;

private:
    // Internal validation helpers
    void InitializeCoreClassPaths();
    void InitializeModuleList();
    FBuild_ClassValidationData InternalValidateClass(const FString& ClassPath);
    FBuild_IntegrationTestResult RunSingleIntegrationTest(const FString& TestName, TFunction<bool()> TestFunction);
    void LogValidationResult(const FBuild_ClassValidationData& Result);
    void LogIntegrationResult(const FBuild_IntegrationTestResult& Result);
    float CalculateModuleScore(const FBuild_ModuleValidationReport& Report);
};