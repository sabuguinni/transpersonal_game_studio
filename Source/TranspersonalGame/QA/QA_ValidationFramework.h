#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationSuite
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString SuiteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float TotalExecutionTime;

    FQA_ValidationSuite()
    {
        SuiteName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunAllValidationSuites();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunValidationSuite(const FString& SuiteName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidateGameplayMechanics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidatePerformance();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationSuite ValidateIntegration();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateClassExists(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateActorInLevel(const FString& ActorName);

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 CountActorsOfClass(UClass* ActorClass);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void SaveValidationResults(const FString& FilePath);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetCurrentFPS();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetMemoryUsageMB();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsPerformanceWithinLimits();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationSuite> ValidationSuites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MinAcceptableFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float MaxAcceptableMemoryMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bAutoRunOnLevelLoad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bGenerateDetailedReports;

private:
    // Internal validation helpers
    FQA_ValidationTest CreateValidationTest(const FString& Name, const FString& Description);
    void ExecuteTest(FQA_ValidationTest& Test, TFunction<bool()> TestFunction);
    void LogValidationResult(const FQA_ValidationTest& Test);
    void UpdateSuiteStatistics(FQA_ValidationSuite& Suite);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationActor : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    class UQA_ValidationFramework* ValidationFramework;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bContinuousValidation;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA")
    void StartContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void StopContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunSingleValidation();

private:
    float ValidationTimer;
    bool bValidationActive;
};