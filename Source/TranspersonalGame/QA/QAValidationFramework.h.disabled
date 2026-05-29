#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QAValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    FQA_SystemReport()
    {
        SystemName = TEXT("");
        ActorCount = 0;
        PassedTests = 0;
        FailedTests = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQAValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQAValidationFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_SystemReport> SystemReports;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 TotalActorsInScene;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 TotalTestsPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 TotalTestsFailed;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float ValidationStartTime;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult TestClassLoading(const FString& ClassName, const FString& ModulePath);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_TestResult TestActorSpawning(UClass* ActorClass, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void SaveValidationResults();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "QA Validation")
    void RunEditorValidation();

private:
    FQA_TestResult CreateTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message);
    void AddSystemReport(const FString& SystemName, const TArray<FQA_TestResult>& Tests);
    int32 CountActorsByKeyword(const FString& Keyword);
};