#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "QA_TestManager.generated.h"

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
class TRANSPERSONALGAME_API AQA_TestManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Testing")
    float TestInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Testing")
    int32 WarningTests;

public:
    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    FQA_TestCase GetTestResult(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "Testing")
    TArray<FQA_TestCase> GetAllTestResults();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void AddTestCase(const FString& Name, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool ValidateGameMode();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    bool ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "Testing")
    void GenerateTestReport();

private:
    FTimerHandle TestTimerHandle;
    
    void ExecuteTest(FQA_TestCase& TestCase);
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdateTestStatistics();
};