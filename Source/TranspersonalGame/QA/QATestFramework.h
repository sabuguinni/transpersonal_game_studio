#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "QATestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Pending     UMETA(DisplayName = "Pending")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
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
        Result = EQA_TestResult::Pending;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 ClassesLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 ClassesTotal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    bool bSystemReady;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FString> CriticalErrors;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        ClassesLoaded = 0;
        ClassesTotal = 0;
        bSystemReady = false;
    }
};

/**
 * QA Test Framework for comprehensive game system validation
 * Provides automated testing for all game systems and integration points
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQATestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // Individual test categories
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestEnvironmentSystems();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateSystemIntegrity(const FString& SystemName, const TArray<FString>& RequiredClasses);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GetTestSummary();

protected:
    // Test data storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_SystemValidation> SystemValidations;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxLightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedReports;

private:
    // Internal test helpers
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void LogTestResult(const FQA_TestCase& TestCase);
    bool IsClassLoaded(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
    
    // Performance monitoring
    float LastFrameTime;
    int32 CurrentActorCount;
    bool bPerformanceWarning;
};