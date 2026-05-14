#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "QA_SystemValidator.generated.h"

/**
 * QA System Validator - Comprehensive testing and validation system
 * Validates all game systems, performance metrics, and integration points
 * Used by QA Agent #18 to ensure system stability and quality
 */

UENUM(BlueprintType)
enum class EQA_ValidationResult : uint8
{
    Pass        UMETA(DisplayName = "Pass"),
    Warning     UMETA(DisplayName = "Warning"), 
    Fail        UMETA(DisplayName = "Fail"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationReport
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

    FQA_ValidationReport()
    {
        TestName = TEXT("");
        Result = EQA_ValidationResult::Pass;
        Details = TEXT("");
        ExecutionTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_PerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 VFXActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CharacterCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    FQA_PerformanceMetrics()
    {
        TotalActorCount = 0;
        VFXActorCount = 0;
        CharacterCount = 0;
        FrameTime = 0.0f;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQA_SystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_SystemValidator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunFullSystemValidation();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ValidateIntegrationPoints();

    // Performance testing
    UFUNCTION(BlueprintCallable, Category = "QA")
    FQA_PerformanceMetrics GetCurrentPerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsPerformanceWithinLimits();

    // Report generation
    UFUNCTION(BlueprintCallable, Category = "QA")
    TArray<FQA_ValidationReport> GetValidationReports() const { return ValidationReports; }

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void GenerateQAReport();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void ClearValidationReports();

    // System status checks
    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetCriticalIssueCount();

    UFUNCTION(BlueprintCallable, Category = "QA")
    int32 GetWarningCount();

protected:
    // Internal validation methods
    void ValidateClassLoading();
    void ValidateActorSpawning();
    void ValidateComponentSystems();
    void ValidateLevelIntegrity();
    void ValidateMemoryUsage();

    // Helper functions
    void AddValidationReport(const FString& TestName, EQA_ValidationResult Result, const FString& Details, float ExecutionTime = 0.0f);
    bool ValidateClassExists(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    TArray<FQA_ValidationReport> ValidationReports;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA", meta = (AllowPrivateAccess = "true"))
    FQA_PerformanceMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    bool bAutoRunValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorCountThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA", meta = (AllowPrivateAccess = "true"))
    float MaxFrameTimeThreshold;

    float LastValidationTime;
    bool bValidationInProgress;
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

    // Manager functions
    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void InitializeQASystem();

    UFUNCTION(BlueprintCallable, Category = "QA", CallInEditor)
    void RunSystemWideValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    bool IsQASystemReady();

    // Global validation controls
    UFUNCTION(BlueprintCallable, Category = "QA")
    void EnableContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void DisableContinuousValidation();

    UFUNCTION(BlueprintCallable, Category = "QA")
    void SetValidationLevel(int32 Level);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QA")
    class UQA_SystemValidator* SystemValidator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    bool bContinuousValidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    int32 ValidationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA")
    float GlobalValidationInterval;

private:
    float LastGlobalValidation;
    bool bQASystemInitialized;
};