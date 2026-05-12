#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_VFXSystemValidator.generated.h"

UENUM(BlueprintType)
enum class EQA_VFXValidationResult : uint8
{
    NotTested       UMETA(DisplayName = "Not Tested"),
    Pass           UMETA(DisplayName = "Pass"),
    Fail           UMETA(DisplayName = "Fail"),
    Critical       UMETA(DisplayName = "Critical Failure")
};

USTRUCT(BlueprintType)
struct FQA_VFXTestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_VFXValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_VFXTestResult()
    {
        TestName = TEXT("");
        Result = EQA_VFXValidationResult::NotTested;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_VFXSystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_VFXSystemValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateVFXImpactManager();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateNiagaraIntegration();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateParticleSystemPerformance();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool ValidateEnvironmentalVFX();

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    void RunFullVFXValidationSuite();

    // Test result access
    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    TArray<FQA_VFXTestResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    bool HasCriticalFailures() const;

    UFUNCTION(BlueprintCallable, Category = "QA|VFX")
    FString GenerateValidationReport() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_VFXTestResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bValidationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float TotalValidationTime;

private:
    void AddTestResult(const FString& TestName, EQA_VFXValidationResult Result, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    bool TestClassLoading(const FString& ClassName);
    bool TestActorSpawning(UClass* ActorClass, const FVector& Location);
    bool TestComponentFunctionality(AActor* TestActor);
};