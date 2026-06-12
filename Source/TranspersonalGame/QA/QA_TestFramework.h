#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "QA_TestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
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
        Result = EQA_TestResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    TArray<FQA_TestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Framework")
    float TestTimeout;

public:
    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void RunSingleTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void ClearTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    TArray<FQA_TestCase> GetTestResults() const;

    UFUNCTION(BlueprintCallable, Category = "QA Framework")
    void AddTestCase(const FString& TestName, const FString& Description);

    // VFX Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    void TestVFXParticleManager();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    void TestNiagaraSystemsLoading();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    void TestVFXIntegrationWithCharacters();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    void TestVFXIntegrationWithDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "QA VFX Tests")
    void TestEnvironmentVFXSystems();

    // Performance Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    void TestActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    void TestNiagaraComponentCount();

    UFUNCTION(BlueprintCallable, Category = "QA Performance Tests")
    void TestMemoryUsage();

    // Integration Testing Functions
    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    void TestCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    void TestDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    void TestWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Integration Tests")
    void TestAudioSystems();

private:
    void LogTestResult(const FString& TestName, EQA_TestResult Result, const FString& Message = TEXT(""));
    
    FDateTime TestStartTime;
    
    void StartTestTimer();
    float GetTestExecutionTime();
};