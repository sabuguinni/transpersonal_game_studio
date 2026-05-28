#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "QA_ValidationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_ValidationResult()
    {
        TestName = TEXT("");
        bPassed = false;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationManager : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // QA Validation Functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateActorCounts();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateClassCompilation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport();

    // Getters
    UFUNCTION(BlueprintPure, Category = "QA")
    TArray<FQA_ValidationResult> GetValidationResults() const { return ValidationResults; }

    UFUNCTION(BlueprintPure, Category = "QA")
    bool GetOverallValidationStatus() const { return bOverallValidationPassed; }

    UFUNCTION(BlueprintPure, Category = "QA")
    int32 GetTotalActorCount() const { return TotalActorCount; }

protected:
    // QA Properties
    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_ValidationResult> ValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bOverallValidationPassed;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    int32 CustomActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float LastValidationTime;

    // Performance Thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxActorCountThreshold = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MinBiomeActorCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float MaxFrameTimeThreshold = 33.33f; // 30 FPS

private:
    // Helper Functions
    FQA_ValidationResult CreateValidationResult(const FString& TestName, bool bPassed, const FString& ErrorMessage = TEXT(""), float ExecutionTime = 0.0f);
    void LogValidationResult(const FQA_ValidationResult& Result);
    void ClearValidationResults();
};