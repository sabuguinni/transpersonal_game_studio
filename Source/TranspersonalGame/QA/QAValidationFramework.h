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
struct FQA_ValidationTest
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
    float ExecutionTimeMs;

    FQA_ValidationTest()
    {
        TestName = TEXT("");
        TestDescription = TEXT("");
        Result = EQA_ValidationResult::Skipped;
        ErrorMessage = TEXT("");
        ExecutionTimeMs = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FQA_SystemReport
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    FString SystemName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    TArray<FQA_ValidationTest> Tests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 PassCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 FailCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 WarningCount;

    FQA_SystemReport()
    {
        SystemName = TEXT("");
        PassCount = 0;
        FailCount = 0;
        WarningCount = 0;
    }
};

/**
 * QA Validation Framework - Comprehensive testing system for TranspersonalGame
 * Validates all game systems, assets, and performance metrics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQAValidationFramework : public AActor
{
    GENERATED_BODY()

public:
    AQAValidationFramework();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullValidation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateDinosaurAssets();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateValidationReport();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_ValidationTest CreateTest(const FString& TestName, const FString& Description);

    UFUNCTION(BlueprintCallable, Category = "QA")
    void CompleteTest(UPARAM(ref) FQA_ValidationTest& Test, EQA_ValidationResult Result, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    float GetMemoryUsagePercent();

protected:
    // Validation data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    TArray<FQA_SystemReport> SystemReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    int32 TotalTests;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    int32 TotalPasses;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    int32 TotalFailures;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    int32 TotalWarnings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    float ValidationStartTime;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    bool bValidationInProgress;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Performance")
    int32 MaxActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Performance")
    float MaxMemoryUsagePercent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA|Performance")
    float MaxFrameTimeMs;

private:
    // Internal validation helpers
    bool ValidateClassLoading(const FString& ClassName, const FString& ClassPath);
    bool ValidateActorSpawning(UClass* ActorClass, const FVector& Location);
    bool ValidateAssetLoading(const FString& AssetPath);
    void LogValidationResult(const FQA_ValidationTest& Test);
};