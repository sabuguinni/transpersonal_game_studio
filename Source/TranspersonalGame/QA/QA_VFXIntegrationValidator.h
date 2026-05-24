#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "QA_VFXIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    FQA_VFXTestCase()
    {
        TestName = TEXT("Unnamed Test");
        TestDescription = TEXT("No description");
        Result = EQA_VFXTestResult::NotTested;
        ExecutionTime = 0.0f;
        ErrorMessage = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct FQA_VFXPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 TotalActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 VFXActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    bool bPerformanceAcceptable;

    FQA_VFXPerformanceMetrics()
    {
        TotalActorCount = 0;
        VFXActorCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        bPerformanceAcceptable = false;
    }
};

/**
 * QA VFX Integration Validator
 * Comprehensive testing system for VFX integration with game systems
 * Validates performance, functionality, and integration quality
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_VFXIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    AQA_VFXIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullVFXValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystemCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXIntegrationWithCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXIntegrationWithAudio();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_VFXPerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceWithinLimits();

    // Test case management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FQA_VFXTestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_VFXTestCase> GetAllTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void SaveValidationReport(const FString& FilePath);

protected:
    // Test case storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_VFXTestCase> TestCases;

    // Performance metrics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Performance")
    FQA_VFXPerformanceMetrics CurrentMetrics;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxAcceptableActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float MaxAcceptableMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float ValidationInterval;

    // Internal state
    UPROPERTY()
    float LastValidationTime;

    UPROPERTY()
    bool bValidationInProgress;

private:
    // Helper functions
    void RunTestCase(const FString& TestName, const FString& Description, TFunction<bool()> TestFunction);
    bool CheckVFXClassExists(const FString& ClassName);
    int32 CountVFXActorsInLevel();
    float GetCurrentFrameRate();
    float GetCurrentMemoryUsage();
    void LogTestResult(const FQA_VFXTestCase& TestCase);
};