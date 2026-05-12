#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "UObject/NoExportTypes.h"
#include "QATestFramework.generated.h"

// QA Test Result Structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA Test")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

// QA Test Categories
UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Functionality  UMETA(DisplayName = "Functionality"),
    VFX            UMETA(DisplayName = "VFX Systems"),
    Audio          UMETA(DisplayName = "Audio Systems"),
    Gameplay       UMETA(DisplayName = "Gameplay"),
    Memory         UMETA(DisplayName = "Memory Usage")
};

// QA Test Priority Levels
UENUM(BlueprintType)
enum class EQA_TestPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"),
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low")
};

/**
 * QA Test Framework Manager
 * Handles automated testing, validation, and quality assurance for the Transpersonal Game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

    // Core Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestsByCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCriticalTests();

    // Performance Testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestResult TestFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestResult TestMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_TestResult TestActorCount();

    // VFX System Testing
    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    FQA_TestResult TestVFXSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    FQA_TestResult TestEnvironmentalVFX();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    FQA_TestResult TestParticleSystemPerformance();

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestResult TestCharacterSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestResult TestDinosaurAIIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FQA_TestResult TestWorldGenerationIntegration();

    // Level Validation
    UFUNCTION(BlueprintCallable, Category = "QA Level")
    FQA_TestResult ValidateLevelIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Level")
    FQA_TestResult ValidateCriticalActors();

    UFUNCTION(BlueprintCallable, Category = "QA Level")
    FQA_TestResult ValidateLightingSetup();

    // Gameplay Testing
    UFUNCTION(BlueprintCallable, Category = "QA Gameplay")
    FQA_TestResult TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Gameplay")
    FQA_TestResult TestSurvivalSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Gameplay")
    FQA_TestResult TestInteractionSystems();

    // Reporting Functions
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    TArray<FQA_TestResult> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    float GetOverallHealthScore() const;

protected:
    // Test Results Storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_TestResult> TestResults;

    // Test Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxAcceptableActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float MaxAcceptableMemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bEnableVerboseLogging;

    // Helper Functions
    FQA_TestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_TestResult& Result);
    UWorld* GetTestWorld() const;
    void ClearTestResults();

private:
    // Internal test tracking
    int32 TotalTestsRun;
    int32 TestsPassed;
    int32 TestsFailed;
    float TotalExecutionTime;
};

/**
 * QA Test Actor - Spawnable actor for in-level testing
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

    // Test Functions
    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void RunActorTests();

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void TestActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Test Actor")
    void TestComponentIntegration();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    class UStaticMeshComponent* TestMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

private:
    float LastTestTime;
    bool bTestsCompleted;
};