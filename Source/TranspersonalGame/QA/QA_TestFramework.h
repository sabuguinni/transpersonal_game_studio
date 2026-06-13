#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "QA_TestFramework.generated.h"

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

USTRUCT(BlueprintType)
struct FQA_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    float TotalExecutionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Report")
    FDateTime ReportTimestamp;

    FQA_ValidationReport()
    {
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
        ReportTimestamp = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_TestFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_TestFramework();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Framework")
    UStaticMeshComponent* TestMarkerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    FQA_ValidationReport CurrentReport;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bLogTestResults;

public:
    virtual void Tick(float DeltaTime) override;

    // Core QA Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    FQA_ValidationReport GetValidationReport() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ClearTestResults();

    // VFX System Testing
    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    EQA_TestResult ValidateVFXSystemManager();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    EQA_TestResult ValidateVFXActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    EQA_TestResult ValidateVFXAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    EQA_TestResult ValidateVFXPerformance();

    // Audio System Testing
    UFUNCTION(BlueprintCallable, Category = "QA Audio Tests")
    EQA_TestResult ValidateAudioManager();

    UFUNCTION(BlueprintCallable, Category = "QA Audio Tests")
    EQA_TestResult ValidateThreatLevelSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Audio Tests")
    EQA_TestResult ValidateAudioSpatialEffects();

    // Character System Testing
    UFUNCTION(BlueprintCallable, Category = "QA Character Tests")
    EQA_TestResult ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "QA Character Tests")
    EQA_TestResult ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Character Tests")
    EQA_TestResult ValidateSurvivalStats();

    // World System Testing
    UFUNCTION(BlueprintCallable, Category = "QA World Tests")
    EQA_TestResult ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA World Tests")
    EQA_TestResult ValidateBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "QA World Tests")
    EQA_TestResult ValidateTerrainSystem();

    // Dinosaur AI Testing
    UFUNCTION(BlueprintCallable, Category = "QA AI Tests")
    EQA_TestResult ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA AI Tests")
    EQA_TestResult ValidateCombatSystem();

    UFUNCTION(BlueprintCallable, Category = "QA AI Tests")
    EQA_TestResult ValidateNPCBehavior();

    // Performance Testing
    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    EQA_TestResult ValidateFrameRate();

    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    EQA_TestResult ValidateMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    EQA_TestResult ValidateActorCount();

    // Integration Testing
    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    EQA_TestResult ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    EQA_TestResult ValidateCrossModuleCommunication();

    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    EQA_TestResult ValidateGameplayFlow();

protected:
    // Helper Functions
    void AddTestResult(const FString& TestName, const FString& Description, EQA_TestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    
    void LogTestResult(const FQA_TestCase& TestCase);
    
    bool IsClassLoaded(const FString& ClassName);
    
    int32 CountActorsOfType(const FString& ActorType);
    
    float MeasureFrameTime();

private:
    FTimerHandle TestTimerHandle;
    float LastTestTime;
    bool bTestsRunning;
};