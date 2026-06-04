#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_VFXTestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotTested,
    Pass,
    Fail,
    Warning
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float ExecutionTime;

    FQA_VFXTestCase()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_VFXTestResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXTestFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXTestFramework();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    TArray<FQA_VFXTestCase> TestCases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestTimeout;

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunSpecificTest(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestScreenShakeSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestNiagaraParticleSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestVFXAudioIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestDamageFlashEffect();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool TestEnvironmentalVFX();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    FQA_VFXTestCase GetTestResult(const FString& TestName);

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ClearAllTestResults();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateTestReport();

protected:
    virtual void BeginPlay() override;

private:
    void InitializeTestCases();
    void LogTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& Message = TEXT(""));
    bool ValidateVFXClass(const FString& ClassName);
    bool ValidateNiagaraSystem(const FString& SystemPath);
};