#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BuildIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Error       UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EBuild_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString Message;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ExecutionTime;

    FBuild_ValidationTest()
    {
        TestName = TEXT("");
        Result = EBuild_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    TArray<FBuild_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 ErrorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FDateTime ReportTimestamp;

    FBuild_IntegrationReport()
    {
        PassCount = 0;
        WarningCount = 0;
        FailCount = 0;
        ErrorCount = 0;
        TotalExecutionTime = 0.0f;
        ReportTimestamp = FDateTime::Now();
    }
};

/**
 * Build Integration Validator - Validates that all game systems integrate correctly
 * Tests module loading, class construction, property access, and cross-system dependencies
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuildIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    ABuildIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationTest ValidateModuleLoading();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationTest ValidateClassConstruction();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationTest ValidatePropertyAccess();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationTest ValidateCrossSystemDependencies();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_ValidationTest ValidateGameplayIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void LogValidationReport(const FBuild_IntegrationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsValidationPassing(const FBuild_IntegrationReport& Report);

protected:
    // Core classes to validate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> CoreClassPaths;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationTimeout;

    // Results storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Results")
    FBuild_IntegrationReport LastReport;

private:
    // Helper functions
    FBuild_ValidationTest CreateTest(const FString& TestName, EBuild_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    bool TryLoadClass(const FString& ClassPath, UClass*& OutClass);
    bool TryConstructCDO(UClass* Class);
    void UpdateReportCounts(FBuild_IntegrationReport& Report);
};