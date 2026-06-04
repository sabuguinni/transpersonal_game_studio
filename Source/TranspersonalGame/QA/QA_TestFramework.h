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

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunCompilationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateActorSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Results")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedLogs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxTestActors;

protected:
    virtual void BeginPlay() override;

private:
    void LogTestResult(const FQA_TestCase& TestCase);
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);
    void CleanupTestActors();

    UPROPERTY()
    TArray<AActor*> SpawnedTestActors;
};