#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "QA_SystemValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    NotTested = 0,
    Passed = 1,
    Failed = 2,
    Warning = 3
};

USTRUCT(BlueprintType)
struct FQA_SystemTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float TestDuration;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorsAffected;

    FQA_SystemTestResult()
    {
        SystemName = TEXT("");
        Result = EQA_ValidationResult::NotTested;
        ErrorMessage = TEXT("");
        TestDuration = 0.0f;
        ActorsAffected = 0;
    }
};

USTRUCT(BlueprintType)
struct FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameRate;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveParticles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    FQA_PerformanceMetrics()
    {
        FrameRate = 0.0f;
        MemoryUsageMB = 0.0f;
        DrawCalls = 0;
        ActiveParticles = 0;
        GPUTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_SystemValidationManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_SystemValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidateAISystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemTestResult ValidateAudioSystems();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool RunPerformanceStressTest(int32 Duration);

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    TArray<FQA_SystemTestResult> RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    bool ValidateSystemIntegration(const FString& SystemA, const FString& SystemB);

    // Automated testing
    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void StartAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    void StopAutomatedTesting();

    UFUNCTION(BlueprintCallable, Category = "QA Automation")
    bool IsAutomatedTestingActive() const { return bAutomatedTestingActive; }

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Reporting")
    void SaveValidationReport(const FString& FilePath);

protected:
    // Test results storage
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_SystemTestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    FQA_PerformanceMetrics LastPerformanceMetrics;

    // Testing configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bRunContinuousValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Config")
    int32 MaxTestIterations;

private:
    // Internal state
    bool bAutomatedTestingActive;
    float TimeSinceLastValidation;
    int32 CurrentTestIteration;

    // Helper functions
    FQA_SystemTestResult CreateTestResult(const FString& SystemName, EQA_ValidationResult Result, const FString& ErrorMsg = TEXT(""));
    void LogTestResult(const FQA_SystemTestResult& Result);
    bool ValidateActorClass(UClass* ActorClass);
    int32 CountActorsOfType(UClass* ActorClass);
    float MeasureSystemPerformance(const FString& SystemName);
};