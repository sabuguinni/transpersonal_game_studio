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
    Critical    UMETA(DisplayName = "Critical"),
    Skipped     UMETA(DisplayName = "Skipped")
};

UENUM(BlueprintType)
enum class EQA_TestCategory : uint8
{
    Performance     UMETA(DisplayName = "Performance"),
    Integration     UMETA(DisplayName = "Integration"),
    Functionality   UMETA(DisplayName = "Functionality"),
    VFX            UMETA(DisplayName = "VFX"),
    Audio          UMETA(DisplayName = "Audio"),
    AI             UMETA(DisplayName = "AI"),
    Physics        UMETA(DisplayName = "Physics"),
    Gameplay       UMETA(DisplayName = "Gameplay")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    EQA_TestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    float ExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
    int32 Priority;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Category = EQA_TestCategory::Functionality;
        Result = EQA_TestResult::Skipped;
        Description = TEXT("");
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
        Priority = 1;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ParticleSystemCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DrawCallCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TriangleCount;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsage = 0.0f;
        ActorCount = 0;
        ParticleSystemCount = 0;
        DrawCallCount = 0.0f;
        TriangleCount = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Test execution functions
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunTestsByCategory(EQA_TestCategory Category);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase RunSingleTest(const FString& TestName);

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateFrameRate(float MinFrameRate = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateMemoryUsage(float MaxMemoryMB = 4096.0f);

    // VFX testing
    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    bool TestVFXSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    bool TestParticleSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA VFX")
    bool TestImpactEffectSystem();

    // AI testing
    UFUNCTION(BlueprintCallable, Category = "QA AI")
    bool TestDinosaurAIBehavior();

    UFUNCTION(BlueprintCallable, Category = "QA AI")
    bool TestNPCInteractions();

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool TestCrossModuleCompatibility();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void GenerateTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    TArray<FQA_TestCase> GetFailedTests();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    float GetTestSuccessRate();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bGenerateDetailedReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float PerformanceTestInterval;

private:
    float LastTestTime;
    float LastPerformanceCheck;
    
    // Internal test functions
    FQA_TestCase TestActorSpawning();
    FQA_TestCase TestComponentRegistration();
    FQA_TestCase TestBlueprintIntegration();
    FQA_TestCase TestAssetLoading();
    FQA_TestCase TestPhysicsSystem();
    FQA_TestCase TestAudioSystem();
    
    void LogTestResult(const FQA_TestCase& TestCase);
    void UpdatePerformanceMetrics();
};