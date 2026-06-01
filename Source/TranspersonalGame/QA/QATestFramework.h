#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 VFXActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float EstimatedRAMUsageGB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    bool bWithinLimits;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        VFXActors = 0;
        EstimatedRAMUsageGB = 0.0f;
        FrameRate = 0.0f;
        bWithinLimits = true;
    }
};

/**
 * QA Test Framework for Transpersonal Game Studio
 * Provides comprehensive testing and validation for all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core testing functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunModuleValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunGameplayTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXValidationTests();

    // Individual test functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateDinosaurLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateModuleClasses();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateGameMode();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateBiomeDistribution();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool CheckPerformanceLimits();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void LogTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    TArray<FQA_TestCase> GetFailedTests();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    int32 GetTotalTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Utility")
    float GetTestSuccessRate() const;

protected:
    // Test storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Data")
    FQA_PerformanceMetrics CurrentMetrics;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxDinosaurLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxRAMUsageGB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MinFrameRate;

    // Helper functions
    void AddTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void StartTestTimer();
    float EndTestTimer();

private:
    float TestStartTime;
};

/**
 * QA Test Actor for in-world testing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunInWorldTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSystems();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(BlueprintReadOnly, Category = "QA Data")
    UQA_TestFramework* TestFramework;

private:
    float LastTestTime;
};