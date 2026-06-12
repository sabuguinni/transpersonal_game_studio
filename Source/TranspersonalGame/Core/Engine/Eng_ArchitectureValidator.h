#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Eng_ArchitectureValidator.generated.h"

UENUM(BlueprintType)
enum class EEng_ValidationResult : uint8
{
    Pass UMETA(DisplayName = "Pass"),
    Warning UMETA(DisplayName = "Warning"),
    Fail UMETA(DisplayName = "Fail")
};

USTRUCT(BlueprintType)
struct FEng_ValidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString TestName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EEng_ValidationResult Result;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString Message;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    float ExecutionTime;

    FEng_ValidationReport()
    {
        TestName = TEXT("");
        Result = EEng_ValidationResult::Pass;
        Message = TEXT("");
        ExecutionTime = 0.0f;
    }
};

/**
 * Engine Architecture Validator - Validates core game architecture integrity
 * Ensures all critical systems are properly configured and functional
 * Used by Engine Architect to maintain system health across development cycles
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitectureValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ArchitectureValidator();

protected:
    virtual void BeginPlay() override;

public:
    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    TArray<FEng_ValidationReport> ValidateGameArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEng_ValidationReport ValidateGameMode();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEng_ValidationReport ValidatePlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEng_ValidationReport ValidateLevelActors();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEng_ValidationReport ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Architecture Validation")
    FEng_ValidationReport ValidateRenderingSystem();

    // System health monitoring
    UFUNCTION(BlueprintCallable, Category = "System Health")
    float GetSystemPerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    float GetFrameRate();

    UFUNCTION(BlueprintCallable, Category = "System Health")
    float GetMemoryUsage();

    // Architecture enforcement
    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    bool EnforceNamingConventions();

    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    bool ValidateModuleDependencies();

    UFUNCTION(BlueprintCallable, Category = "Architecture Enforcement")
    bool CheckForArchitectureViolations();

protected:
    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bLogValidationResults;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MinAcceptableFrameRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxAcceptableActorCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxAcceptableMemoryUsage;

    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    TArray<FEng_ValidationReport> LastValidationResults;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    bool bIsValidationInProgress;

private:
    // Internal validation helpers
    FEng_ValidationReport CreateValidationReport(const FString& TestName, EEng_ValidationResult Result, const FString& Message);
    void LogValidationReport(const FEng_ValidationReport& Report);
    float MeasureExecutionTime(TFunction<void()> TestFunction);

    // Timer handle for periodic validation
    FTimerHandle ValidationTimerHandle;
    void PerformPeriodicValidation();
};