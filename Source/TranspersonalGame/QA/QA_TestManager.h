#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
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

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Test")
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

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DinosaurCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 PropCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 LightCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    FQA_PerformanceMetrics()
    {
        TotalActors = 0;
        DinosaurCount = 0;
        PropCount = 0;
        LightCount = 0;
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
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
    void RunGameplayTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateActorLimits();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateBiomeDistribution();

    // Test result access
    UFUNCTION(BlueprintPure, Category = "QA Testing")
    TArray<FQA_TestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintPure, Category = "QA Testing")
    FQA_PerformanceMetrics GetPerformanceMetrics() const { return PerformanceMetrics; }

    UFUNCTION(BlueprintPure, Category = "QA Testing")
    bool AreAllTestsPassing() const;

    UFUNCTION(BlueprintPure, Category = "QA Testing")
    int32 GetFailedTestCount() const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExportTestReport(const FString& FilePath);

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Testing")
    FQA_PerformanceMetrics PerformanceMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxActorLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxDinosaurLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings")
    int32 MaxPropsPerBiome;

private:
    // Internal test functions
    void TestCharacterSpawning();
    void TestMovementSystem();
    void TestDinosaurAI();
    void TestVFXSystems();
    void TestAudioSystems();
    void TestWorldGeneration();
    void TestCombatSystems();
    void TestQuestSystems();
    void TestUIElements();
    void TestSaveLoadSystem();

    // Performance monitoring
    void UpdatePerformanceMetrics();
    void CheckMemoryUsage();
    void CheckFrameRate();

    // Utility functions
    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""));
    TArray<AActor*> GetActorsInBiome(const FVector& BiomeCenter, float Radius = 15000.0f);
    
    float LastTestTime;
    bool bTestsRunning;
};