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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString TestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    EQA_ValidationResult Result;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ErrorMessage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    int32 WarningCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
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
 * QA Validation Framework - Comprehensive testing system for all game components
 * Provides automated validation of VFX, character systems, world generation, physics, and audio
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_ValidationFramework : public UObject
{
    GENERATED_BODY()

public:
    UQA_ValidationFramework();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateAudioSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_SystemValidationReport ValidateBiomePopulation();

    // Comprehensive validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    TArray<FQA_SystemValidationReport> RunFullValidationSuite();

    // Performance validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_ValidationTest ValidatePerformanceMetrics();

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_ValidationTest ValidateAssetIntegrity();

    // Scene validation
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    FQA_ValidationTest ValidateSceneSetup();

protected:
    // Helper functions
    FQA_ValidationTest CreateTest(const FString& TestName, const FString& Description);
    void SetTestResult(FQA_ValidationTest& Test, EQA_ValidationResult Result, const FString& ErrorMessage = TEXT(""));
    void StartTestTimer(FQA_ValidationTest& Test);
    void EndTestTimer(FQA_ValidationTest& Test);

    // Validation data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_SystemValidationReport> ValidationReports;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    int32 TotalTestsRun;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    int32 TotalTestsPassed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    int32 TotalTestsFailed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation", meta = (AllowPrivateAccess = "true"))
    float TotalValidationTime;

private:
    // Timer tracking
    double TestStartTime;
    
    // Critical system requirements
    static const int32 MinimumActorsPerBiome = 500;
    static const int32 MaximumRecommendedActors = 20000;
    static const float MaximumValidationTime = 30.0f;
};