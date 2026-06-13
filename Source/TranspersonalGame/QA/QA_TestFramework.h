#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Compilation     UMETA(DisplayName = "Compilation"),
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    VFX             UMETA(DisplayName = "VFX"),
    Audio           UMETA(DisplayName = "Audio"),
    AI              UMETA(DisplayName = "AI"),
    WorldGen        UMETA(DisplayName = "World Generation")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Compilation;
        Result = EQA_TestResult::NotRun;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 VegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    int32 VFXSystemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health")
    bool bSystemReady;

    FQA_SystemHealth()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        VegetationCount = 0;
        VFXSystemCount = 0;
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
        bSystemReady = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    // System validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemHealth GetSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateClassLoading();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateSceneIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    // VFX specific validation
    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    bool TestParticleSpawning();

    // Dinosaur AI validation
    UFUNCTION(BlueprintCallable, Category = "QA AI")
    bool ValidateDinosaurBehavior();

    UFUNCTION(BlueprintCallable, Category = "QA AI")
    bool TestDinosaurMovement();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GetTestSummary();

    // Test result accessors
    UFUNCTION(BlueprintCallable, Category = "QA Results")
    TArray<FQA_TestCase> GetAllTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Results")
    TArray<FQA_TestCase> GetFailedTests() const;

    UFUNCTION(BlueprintCallable, Category = "QA Results")
    float GetOverallSuccessRate() const;

protected:
    // Test case storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Data")
    FQA_SystemHealth CurrentSystemHealth;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedReports;

private:
    // Internal test functions
    void InitializeTestSuite();
    void ExecuteCompilationTests();
    void ExecutePerformanceTests();
    void ExecuteIntegrationTests();
    void ExecuteGameplayTests();
    void ExecuteVFXTests();
    void ExecuteAudioTests();
    void ExecuteAITests();
    void ExecuteWorldGenTests();

    // Helper functions
    FQA_TestCase CreateTestCase(const FString& Name, EQA_TestCategory Category, const FString& Description);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateSystemHealth();

    // Timing
    float LastTestTime;
    bool bTestsInitialized;
};