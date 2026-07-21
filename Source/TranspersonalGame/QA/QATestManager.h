#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QATestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
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
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 StaticMeshActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 SkeletalMeshActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 LightActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ParticleActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DinosaurActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        StaticMeshActors = 0;
        SkeletalMeshActors = 0;
        LightActors = 0;
        ParticleActors = 0;
        DinosaurActors = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * QA Test Manager - Automated testing and validation system
 * Ensures game quality and performance standards are maintained
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestManager : public AActor
{
    GENERATED_BODY()

public:
    AQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunGameplayTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // Individual test functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestDinosaurCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestPlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase TestAudioSystems();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool IsPerformanceAcceptable();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateModuleIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAssetReferences();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateGameplayFlow();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GetTestSummary();

protected:
    // Test data storage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Data")
    FQA_PerformanceMetrics CurrentMetrics;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MinFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxMemoryUsageMB;

    // Test execution control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Control")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Control")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Control")
    bool bGenerateReports;

private:
    // Internal test helpers
    void InitializeTestSuite();
    void UpdatePerformanceMetrics();
    void LogTestResult(const FQA_TestCase& TestCase);
    void SaveTestResults();

    // Timer for periodic testing
    FTimerHandle TestTimerHandle;
    float LastTestTime;
};