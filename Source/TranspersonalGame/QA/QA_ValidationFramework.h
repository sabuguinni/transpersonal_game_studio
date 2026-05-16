#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct FQA_TestResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }

    FQA_TestResult(const FString& InTestName, EQA_ValidationResult InResult, const FString& InMessage, float InTime)
        : TestName(InTestName), Result(InResult), Message(InMessage), ExecutionTime(InTime)
    {
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 CompiledClasses;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 BiomesPopulated;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bLightingValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPhysicsValid;

    FQA_SystemHealth()
    {
        TotalActors = 0;
        CompiledClasses = 0;
        MemoryUsagePercent = 0.0f;
        BiomesPopulated = 0;
        bLightingValid = false;
        bPhysicsValid = false;
    }
};

/**
 * QA Validation Framework for automated testing and quality assurance
 * Provides comprehensive testing of game systems, performance, and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateSystemHealth();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateLightingAndAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePhysicsAndCollision();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport();

    // Getters for test results
    UFUNCTION(BlueprintPure, Category = "QA")
    const TArray<FQA_TestResult>& GetTestResults() const { return TestResults; }

    UFUNCTION(BlueprintPure, Category = "QA")
    const FQA_SystemHealth& GetSystemHealth() const { return SystemHealth; }

    UFUNCTION(BlueprintPure, Category = "QA")
    int32 GetPassedTestCount() const;

    UFUNCTION(BlueprintPure, Category = "QA")
    int32 GetFailedTestCount() const;

    UFUNCTION(BlueprintPure, Category = "QA")
    float GetOverallSuccessRate() const;

protected:
    // Test result storage
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FQA_SystemHealth SystemHealth;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FDateTime LastValidationTime;

    // Helper functions
    void AddTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Message, float ExecutionTime = 0.0f);
    void ClearTestResults();
    bool ValidateClassCompilation(const FString& ClassName);
    int32 CountActorsInBiome(const FVector& BiomeCenter, float Radius = 10000.0f);
    bool CheckActorCollision(AActor* Actor);
    void LogValidationMessage(const FString& Message, EQA_ValidationResult ResultType = EQA_ValidationResult::Pass);

private:
    // Internal validation state
    bool bValidationInProgress;
    float ValidationStartTime;
};