#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "QATestManager.generated.h"

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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_TestResult::Pending;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 TotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 StaticMeshCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 LightCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        StaticMeshCount = 0;
        LightCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQATestManager : public AActor
{
    GENERATED_BODY()

public:
    AQATestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxDinosaurLimit;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunActorCountTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunBiomeDistributionTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunModuleLoadTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_PerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool IsSystemHealthy() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    void ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Testing", CallInEditor = true)
    void GenerateQAReport();

private:
    FTimerHandle TestTimerHandle;
    float LastTestTime;

    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    void UpdatePerformanceMetrics();
    void LogTestResult(const FQA_TestCase& TestCase);
    int32 CountActorsOfType(const FString& TypeName);
    bool ValidateBiomeDistribution();
    bool ValidateModuleIntegrity();
};