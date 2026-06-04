#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "QA_VFXTestFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXTestResult : uint8
{
    NotTested = 0,
    Pass = 1,
    Fail = 2,
    Warning = 3
};

USTRUCT(BlueprintType)
struct FQA_VFXTestCase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    EQA_VFXTestResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float ExecutionTime;

    FQA_VFXTestCase()
    {
        TestName = TEXT("Unnamed Test");
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

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === VFX TESTING METHODS ===
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void RunAllVFXTests();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestNiagaraSystemSpawning();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestCombatParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestEnvironmentalVFX();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void TestVFXPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void ValidateVFXIntegration();

    // === RESULTS AND REPORTING ===
    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    TArray<FQA_VFXTestCase> GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    void GenerateVFXTestReport();

    UFUNCTION(BlueprintCallable, Category = "QA Testing")
    bool AreAllTestsPassing() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA Testing")
    TArray<FQA_VFXTestCase> TestResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    bool bAutoRunTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Testing")
    float TestInterval;

    // === VFX REFERENCES FOR TESTING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    TArray<TSoftObjectPtr<UNiagaraSystem>> TestNiagaraSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX Testing")
    TArray<AActor*> TestActors;

private:
    void AddTestResult(const FString& TestName, EQA_VFXTestResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void LogTestResult(const FQA_VFXTestCase& TestCase);
    float TestStartTime;
};