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
struct FQA_ValidationTest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString TestDescription;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    EQA_ValidationResult Result;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString ErrorMessage;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float ExecutionTime;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    FString SystemName;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    int32 WarningCount;

    UPROPERTY(BlueprintReadWrite, Category = "QA")
    float TotalExecutionTime;

    FQA_SystemValidationReport()
    {
        SystemName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
        TotalExecutionTime = 0.0f;
    }
};

/**
 * QA Validation Framework - Automated testing system for TranspersonalGame
 * Validates all critical systems: VFX, Physics, Character, Biomes, Audio, etc.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation methods
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_SystemValidationReport ValidateCrowdSimulation();

    // Comprehensive validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_SystemValidationReport> RunFullValidationSuite();

    // Bridge health check
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool ValidateBridgeHealth();

    // Level integrity check
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateLevelIntegrity();

    // Class loading validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateClassLoading(const FString& ClassName);

    // Actor count validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidateActorCounts();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest ValidatePerformanceMetrics();

protected:
    // Internal validation helpers
    FQA_ValidationTest CreateTest(const FString& Name, const FString& Description);
    void SetTestResult(FQA_ValidationTest& Test, EQA_ValidationResult Result, const FString& ErrorMsg = TEXT(""));
    void StartTestTimer(FQA_ValidationTest& Test);
    void EndTestTimer(FQA_ValidationTest& Test);

    // System validation helpers
    bool ValidateClassExists(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
    bool ValidateActorInBiome(const FVector& BiomeCenter, float Radius, int32 MinActors);

private:
    // Timing for performance tests
    double TestStartTime;
    
    // Critical class names for validation
    TArray<FString> CriticalClasses;
    
    // Biome definitions for validation
    TMap<FString, FVector> BiomeLocations;
    
    // Performance thresholds
    float MaxFrameTime;
    int32 MaxActorCount;
    float MaxMemoryUsage;
};