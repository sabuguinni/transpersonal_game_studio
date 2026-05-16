#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QAValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Fail        UMETA(DisplayName = "Fail"),
    Warning     UMETA(DisplayName = "Warning"),
    Skipped     UMETA(DisplayName = "Skipped")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ExecutionTimeSeconds;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTimeSeconds = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailedTests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bSystemHealthy;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        PassedTests = 0;
        FailedTests = 0;
        bSystemHealthy = false;
    }
};

/**
 * QA Validation Framework for automated testing of TranspersonalGame systems
 * Provides comprehensive validation of game systems, actors, and performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQAValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQAValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor = true)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidateLightingAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidation ValidatePerformanceMetrics();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    void GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy(const FQA_SystemValidation& SystemValidation);

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<AActor*> GetActorsInBiome(FVector BiomeCenter, float Radius);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    TArray<FQA_SystemValidation> ValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bVerboseLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float ValidationTimeoutSeconds;

    // Helper functions
    FQA_ValidationTest CreateTest(const FString& TestName, const FString& Description);
    void LogValidationResult(const FQA_ValidationTest& Test);
    bool ValidateClassExists(const FString& ClassName);
    bool ValidateActorComponent(AActor* Actor, UClass* ComponentClass);
};