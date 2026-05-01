#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "QATestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float TotalExecutionTime;

    FQA_ValidationReport()
    {
        TotalTests = 0;
        PassedTests = 0;
        FailedTests = 0;
        TotalExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQATestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQATestManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationReport RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateMapState();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateDinosaurPlaceholders();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateVFXSystems();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult ValidateMemoryUsage();

    // Integration tests
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult TestSurvivalStats();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_TestResult TestWorldGeneration();

    // Reporting functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport(const FQA_ValidationReport& Report);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void LogTestResult(const FQA_TestResult& Result);

private:
    UPROPERTY()
    TArray<FQA_TestResult> CurrentTestResults;

    UPROPERTY()
    float ValidationStartTime;

    // Helper functions
    FQA_TestResult CreateTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    
    bool ValidateActorExists(UClass* ActorClass, const FString& ActorName);
    
    int32 CountActorsOfType(UClass* ActorClass);
    
    bool CheckComponentOnActor(AActor* Actor, UClass* ComponentClass);
};