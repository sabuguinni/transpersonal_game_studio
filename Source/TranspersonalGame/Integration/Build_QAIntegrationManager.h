#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Build_QAIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_QATestResult
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

    FBuild_QATestResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_QATestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PassedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedTests;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildTimestamp;

    FBuild_IntegrationReport()
    {
        TotalTests = 0;
        PassedTests = 0;
        FailedTests = 0;
        bBuildSuccessful = false;
        BuildTimestamp = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABuild_QAIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ABuild_QAIntegrationManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Integration")
    float TestInterval;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void RunQAIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void ValidateBuildIntegrity();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    FBuild_IntegrationReport GetIntegrationReport() const;

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool IsSystemHealthy() const;

    UFUNCTION(CallInEditor, Category = "QA Integration")
    void EditorRunIntegrationTests();

private:
    void AddTestResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void GenerateReport();
    void LogIntegrationStatus();

    float LastTestTime;
    bool bTestsRunning;
};