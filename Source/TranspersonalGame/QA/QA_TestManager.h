#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Pending     UMETA(DisplayName = "Pending")
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
        Result = EQA_TestResult::Pending;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    int32 WarningTests;

public:
    virtual void Tick(float DeltaTime) override;

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
    void TestDinosaurSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestAssetLoading();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Testing")
    void RunEditorTests();

private:
    FTimerHandle TestTimerHandle;
    int32 CurrentTestIndex;

    void ExecuteNextTest();
    void LogTestResult(const FQA_TestCase& TestCase);
    bool ValidateActorCount();
    bool ValidateClassCompilation();
    bool ValidateAssetIntegrity();
};

#include "QA_TestManager.generated.h"