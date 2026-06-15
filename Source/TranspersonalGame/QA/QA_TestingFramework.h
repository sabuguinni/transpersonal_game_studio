#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QA_TestingFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    NotRun      UMETA(DisplayName = "Not Run"),
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Error       UMETA(DisplayName = "Error")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
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
class TRANSPERSONALGAME_API UQA_TestingFramework : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQA_TestingFramework();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestByName(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void AddTestCase(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    int32 WarningTests;

private:
    void InitializeTestCases();
    void LogTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
};