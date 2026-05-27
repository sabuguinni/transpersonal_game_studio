#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "QATestManager.generated.h"

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
        Result = EQA_TestResult::NotRun;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunMemoryTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAssetValidationTests();

    // Test result functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    float GetOverallHealthScore() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_TestCase> GetFailedTests() const;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateClassLoading(const FString& ClassName);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateAssetLoading(const FString& AssetPath);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    int32 CountActorsInBiome(FVector BiomeCenter, float Radius);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateBiomePopulation();

protected:
    // Test case storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    float OverallHealthScore;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    int32 TotalTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    int32 PassedTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results")
    int32 FailedTests;

    // Test configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTestsOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedLogs;

    // Visual indicator
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* IndicatorMesh;

private:
    // Internal test functions
    void TestClassLoading();
    void TestAssetLoading();
    void TestActorSpawning();
    void TestComponentSystems();
    void TestBiomeValidation();
    void TestMemoryManagement();
    void TestPerformanceMetrics();

    // Helper functions
    void AddTestCase(const FString& Name, const FString& Description, EQA_TestResult Result, const FString& Error = TEXT(""), float ExecutionTime = 0.0f);
    void UpdateHealthScore();
    void UpdateIndicatorColor();

    // Timer handle for periodic testing
    FTimerHandle TestTimerHandle;
};