#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationManager.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass,
    Warning,
    Fail,
    Critical
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString TestName;

    UPROPERTY(BlueprintReadOnly)
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly)
    FString Message;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationManager();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunAllValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunClassLoadingTests();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunMapValidationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunPerformanceTests();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunIntegrationTests();

    UFUNCTION(BlueprintCallable, Category = "QA Validation", CallInEditor = true)
    void GenerateValidationReport();

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadWrite, Category = "QA Settings")
    bool bAutoRunOnBeginPlay;

    UPROPERTY(BlueprintReadWrite, Category = "QA Settings")
    float PerformanceTargetFPS;

    UPROPERTY(BlueprintReadWrite, Category = "QA Settings")
    int32 MaxAllowedActors;

protected:
    virtual void BeginPlay() override;

private:
    void AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    
    bool ValidateClassExists(const FString& ClassPath);
    bool ValidateActorCount();
    bool ValidateEssentialActors();
    bool ValidatePlayerStart();
    bool ValidateLighting();
    
    float MeasureFrameTime();
    
    TArray<FString> CoreClassPaths;
    TArray<FString> EssentialActorTypes;
};