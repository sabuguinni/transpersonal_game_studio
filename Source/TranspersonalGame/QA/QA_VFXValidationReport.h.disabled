#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_VFXValidationReport.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString Details;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTimeMs;

    FQA_VFXTestCase()
    {
        TestName = TEXT("");
        Result = EQA_VFXTestResult::NotTested;
        Details = TEXT("");
        ExecutionTimeMs = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXValidationReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXValidationReport();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    TArray<FQA_VFXTestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    int32 WarningTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Validation")
    FDateTime TestTimestamp;

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunVFXValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void AddTestCase(const FString& TestName, EQA_VFXTestResult Result, const FString& Details, float ExecutionTime);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GenerateReportSummary();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateFireEffectSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateWaterSplashSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void LogValidationResults();

private:
    void CalculateTestStatistics();
    bool TestActorSpawning(UClass* ActorClass, const FString& TestName);
    bool TestComponentFunctionality(AActor* TestActor, const FString& ComponentName);
};