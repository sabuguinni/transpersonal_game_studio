#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "QA_ValidationFramework.generated.h"

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    NotTested   UMETA(DisplayName = "Not Tested")
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
    FString Details;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    float ExecutionTime;

    FQA_TestResult()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::NotTested;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemValidation
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    TArray<FQA_TestResult> TestResults;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 PassCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 WarningCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA")
    int32 FailCount;

    FQA_SystemValidation()
    {
        SystemName = TEXT("");
        PassCount = 0;
        WarningCount = 0;
        FailCount = 0;
    }
};

/**
 * QA Validation Framework - Comprehensive testing system for all game components
 * Validates character systems, VFX, physics, biomes, dinosaurs, and performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQA_ValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQA_ValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation properties
    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    TArray<FQA_SystemValidation> SystemValidations;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "QA Validation")
    bool bAllCriticalSystemsPass;

    // Biome validation
    UPROPERTY(BlueprintReadOnly, Category = "QA Biomes")
    TMap<FString, int32> BiomeActorCounts;

    UPROPERTY(BlueprintReadOnly, Category = "QA Biomes")
    int32 MinimumActorsPerBiome;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxMemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Performance")
    float MaxFrameTime;

    // Validation methods
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateDinosaurSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidatePerformanceSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidation ValidateNavigationSystem();

    // Utility methods
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void SaveValidationReport(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool CheckBiomePopulation(const FString& BiomeName, const FVector& BiomeCenter, float BiomeRange);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<AActor*> GetActorsInBiome(const FVector& BiomeCenter, float BiomeRange);

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    float GetCurrentMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    void LogTestResult(const FQA_TestResult& TestResult);

private:
    // Internal validation helpers
    FQA_TestResult CreateTestResult(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    
    void UpdateSystemCounts(FQA_SystemValidation& SystemValidation);
    
    bool ValidateClassExists(const FString& ClassName);
    
    TArray<AActor*> GetActorsOfType(const FString& ActorType);
    
    FString GetBiomeForLocation(const FVector& Location);
};