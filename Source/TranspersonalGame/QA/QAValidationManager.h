#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QAValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Connectivity    UMETA(DisplayName = "Connectivity"),
    ClassLoading    UMETA(DisplayName = "Class Loading"),
    ActorSpawning   UMETA(DisplayName = "Actor Spawning"),
    LevelAssets     UMETA(DisplayName = "Level Assets"),
    Blueprint       UMETA(DisplayName = "Blueprint Integration"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Gameplay        UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FDateTime Timestamp;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Connectivity;
        Result = EQA_ValidationResult::Pass;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 CriticalCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bBuildApproved;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString BuildStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FDateTime ReportTimestamp;

    FQA_ValidationReport()
    {
        PassCount = 0;
        WarningCount = 0;
        FailCount = 0;
        CriticalCount = 0;
        bBuildApproved = false;
        BuildStatus = TEXT("Pending");
        ReportTimestamp = FDateTime::Now();
    }
};

/**
 * QA Validation Manager - Comprehensive testing and validation system
 * Handles automated testing, performance validation, and build approval
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQAValidationManager : public UObject
{
    GENERATED_BODY()

public:
    UQAValidationManager();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateConnectivity();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateLevelAssets();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateBlueprintIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_ValidationTest ValidateIntegration();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool CheckPerformanceThresholds();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetLightCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetFrameTime();

    // Build approval system
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    bool ApproveBuild(const FQA_ValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void BlockBuild(const FString& Reason);

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport(const FQA_ValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogValidationResult(const FQA_ValidationTest& Test);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FString> CoreClassNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 MaxLightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bStrictValidation;

private:
    void InitializeCoreClasses();
    FQA_ValidationTest CreateTest(const FString& Name, EQA_TestCategory Category);
    void UpdateReportCounts(FQA_ValidationReport& Report);
};