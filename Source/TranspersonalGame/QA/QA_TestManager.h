#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_TestManager.generated.h"

UENUM(BlueprintType)
enum class EQA_TestResult : uint8
{
    Pass,
    Fail,
    Warning,
    Skipped
};

USTRUCT(BlueprintType)
struct FQA_TestCase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_TestResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Description;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestCase()
    {
        TestName = TEXT("");
        Result = EQA_TestResult::Skipped;
        Description = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_TestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_TestManager();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunAllTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void RunStabilityTests();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void ValidateGameplayElements();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateQAReport();

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestCase> TestResults;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 MaxActorCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 MaxDinosaurCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float TargetFrameRate;

protected:
    virtual void BeginPlay() override;

private:
    void AddTestResult(const FString& TestName, EQA_TestResult Result, const FString& Description, float ExecutionTime = 0.0f);
    
    bool ValidateClassLoading();
    bool ValidateMapActors();
    bool ValidatePlayerSetup();
    bool ValidateLightingSetup();
    bool ValidatePerformanceMetrics();
};