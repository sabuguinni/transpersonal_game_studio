#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "QA_SystemIntegrationValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"),
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test Case")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Description = TEXT("");
        Result = EQA_ValidationResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    bool bIsInitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    int32 ActiveComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    float MemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Status")
    TArray<FQA_TestCase> TestResults;

    FQA_SystemStatus()
    {
        SystemName = TEXT("");
        bIsLoaded = false;
        bIsInitialized = false;
        ActiveComponents = 0;
        MemoryUsageMB = 0.0f;
    }
};

/**
 * QA System Integration Validator
 * Comprehensive testing framework for validating all game systems integration
 * Ensures cross-system compatibility and performance standards
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_SystemIntegrationValidator : public AActor
{
    GENERATED_BODY()

public:
    AQA_SystemIntegrationValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidateCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void ValidatePerformanceMetrics();

    // Test case management
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_TestCase CreateTestCase(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ExecuteTestCase(UPARAM(ref) FQA_TestCase& TestCase, const FString& TestFunction);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void LogTestResult(const FQA_TestCase& TestCase);

    // System status monitoring
    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    FQA_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    TArray<FQA_SystemStatus> GetAllSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "QA Monitoring")
    void GenerateQAReport();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateFrameRate(float MinFPS = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateMemoryUsage(float MaxMemoryMB = 4096.0f);

    UFUNCTION(BlueprintCallable, Category = "QA Performance")
    bool ValidateLoadTimes(float MaxLoadTimeSeconds = 10.0f);

    // Integration testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestVFXAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestCharacterWorldInteraction();

    UFUNCTION(BlueprintCallable, Category = "QA Integration")
    void TestCrowdVFXIntegration();

protected:
    // Test execution helpers
    void ExecuteTestWithTimeout(TFunction<void()> TestFunction, float TimeoutSeconds, FQA_TestCase& TestCase);
    
    void ValidateClassExists(const FString& ClassName, FQA_TestCase& TestCase);
    
    void ValidateActorSpawning(UClass* ActorClass, FQA_TestCase& TestCase);
    
    void ValidateComponentFunctionality(UActorComponent* Component, FQA_TestCase& TestCase);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Components", meta = (AllowPrivateAccess = "true"))
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunValidationOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings", meta = (AllowPrivateAccess = "true"))
    bool bLogDetailedResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Settings", meta = (AllowPrivateAccess = "true"))
    bool bGenerateReportOnCompletion;

    // Test results storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_SystemStatus> SystemStatuses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_TestCase> AllTestResults;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    int32 TotalTestsRun;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    int32 TestsPassed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    int32 TestsFailed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Results", meta = (AllowPrivateAccess = "true"))
    float TotalExecutionTime;

    // Validation state
    float LastValidationTime;
    bool bValidationInProgress;
    
    // Performance tracking
    float CurrentFPS;
    float CurrentMemoryUsage;
    float LastFrameTime;
};