#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
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
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FString ReportName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    float TotalExecutionTime;

    FQA_ValidationReport()
    {
        ReportName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Test Framework - Comprehensive testing and validation system
 * Validates all game systems, performance, and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAudioSystems();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXCharacterIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestWorldVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestAudioVFXIntegration();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestLoadTimes();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void SaveValidationReport(const FQA_ValidationReport& Report, const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_ValidationReport LoadValidationReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResult(const FQA_TestCase& TestCase);

protected:
    // Internal validation helpers
    bool ValidateClassExists(const FString& ClassName);
    bool ValidateAssetExists(const FString& AssetPath);
    bool ValidateActorSpawning(UClass* ActorClass);
    float MeasureFrameRate(float Duration = 5.0f);
    float MeasureMemoryUsage();

    // Test data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    TArray<FString> RequiredClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    TArray<FString> RequiredAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    float MaxMemoryUsage;

private:
    // Internal state
    FDateTime TestStartTime;
    FQA_ValidationReport CurrentReport;
};

/**
 * QA Test Actor - Spawnable actor for in-world testing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestActor();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestSequence();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestAudioPlayback();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class UStaticMeshComponent* TestMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

private:
    float LastTestTime;
    int32 TestIndex;
};