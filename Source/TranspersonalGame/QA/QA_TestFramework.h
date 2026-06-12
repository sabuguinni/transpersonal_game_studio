#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
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
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
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
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    // System validation tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult ValidatePerformance();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    EQA_TestResult TestEnvironmentalEffects();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetMemoryUsage();

    // Test result accessors
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallTestScore() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bLogTestResults;

private:
    float LastTestTime;
    
    // Helper functions
    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void LogTestResult(const FQA_TestCase& TestCase);
    bool IsClassLoaded(const FString& ClassName);
    AActor* SpawnTestActor(UClass* ActorClass, const FVector& Location);
};