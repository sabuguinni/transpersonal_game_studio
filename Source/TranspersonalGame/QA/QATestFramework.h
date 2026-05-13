#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "QATestFramework.generated.h"

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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

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
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // Test case management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    // Specific test implementations
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestPerformanceMetrics();

protected:
    // Test data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    bool bTestsRunning;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    float TotalTestTime;

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

private:
    // Internal test helpers
    void LogTestResult(const FString& TestName, bool bPassed, const FString& ErrorMsg = TEXT(""));
    void UpdateTestCase(const FString& TestName, EQA_TestResult Result, const FString& ErrorMsg = TEXT(""));
    bool ValidateActorSpawning();
    bool ValidateClassLoading();
    bool ValidateComponentSystems();
};