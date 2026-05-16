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
    Pending     UMETA(DisplayName = "Pending")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_TestResult
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
        Result = EQA_ValidationResult::Pending;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_BiomeValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    bool bMeetsPopulationTarget;

    FQA_BiomeValidation()
    {
        BiomeName = TEXT("");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        bMeetsPopulationTarget = false;
    }
};

/**
 * QA Validation Framework for automated testing and quality assurance
 * Provides comprehensive testing of game systems, performance, and integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_TestResult> RunFullValidationSuite();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateBridgeConnection();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    TArray<FQA_BiomeValidation> ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidateLightingAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    FQA_TestResult ValidatePerformanceMetrics();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetSystemMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float CalculateSystemHealthScore();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_BiomeValidation> LastBiomeValidation;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float LastHealthScore;

private:
    // Internal validation helpers
    bool ValidateClassLoading(const FString& ClassName, FString& OutMessage);
    bool ValidateActorSpawning(UClass* ActorClass, FString& OutMessage);
    bool CheckBiomeActorDistribution(const FVector& BiomeCenter, float Radius, int32& OutActorCount);
    bool ValidateAssetImportCapability(FString& OutMessage);
    bool CheckLightingConfiguration(FString& OutMessage);
    bool ValidateMemoryUsage(FString& OutMessage);
};