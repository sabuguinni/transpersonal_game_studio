#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
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
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TargetFrameRate;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunBiomeDistributionTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunModuleCompilationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAssetValidationTest();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CreateTestCase(const FString& Name, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResult(const FQA_TestCase& TestCase);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    int32 GetActorCountInBiome(const FVector& BiomeCenter, float Radius);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool CheckCriticalGameObjects();

private:
    FTimerHandle TestTimerHandle;
    
    void ScheduleNextTest();
    bool ValidateAssetPath(const FString& AssetPath);
    float MeasureFrameRate();
};