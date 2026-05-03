#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
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

/**
 * QA Test Manager - Automated testing system for Transpersonal Game
 * Validates compilation, actor spawning, component functionality, and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQATestManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Test execution
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCompilationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunActorSpawnTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunComponentTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    // Test results
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetTestSuccessRate() const;

    // Validation utilities
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateClassCompilation(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateActorSpawn(UClass* ActorClass, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateQAReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    bool bTestsRunning;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    float TotalTestTime;

private:
    // Internal test methods
    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void ClearTestResults();
    
    // Critical class validation
    void TestCriticalClasses();
    void TestCharacterSystem();
    void TestWorldGeneration();
    void TestVFXSystem();
    void TestAudioSystem();
    
    // Map validation
    void TestMapIntegrity();
    void TestLightingSetup();
    void TestPlayerSpawn();
    void TestDinosaurActors();
    
    // Performance validation
    void TestFrameRate();
    void TestMemoryUsage();
    void TestAssetLoading();
};