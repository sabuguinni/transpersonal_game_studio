#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
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

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    UStaticMeshComponent* TestMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 WarningTests;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FString& Name, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase GetTestResult(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    // Core system validation tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidatePerformance();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestCharacterVFXIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestWorldAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestDinosaurAIIntegration();

private:
    void InitializeDefaultTests();
    void UpdateTestStatistics();
    void LogTestResult(const FQA_TestCase& TestCase);

    FTimerHandle TestTimerHandle;
};