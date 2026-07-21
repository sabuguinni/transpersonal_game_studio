#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "QA_CriticalSystemValidator.generated.h"

/**
 * QA Critical System Validator - Emergency validation system for detecting and reporting
 * critical failures in the TranspersonalGame systems after bridge failures or crashes.
 * This system provides automated validation of core game systems and generates
 * incident reports for critical failures.
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQA_CriticalSystemValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UQA_CriticalSystemValidator();

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateGameSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidateWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "QA Validation")
    bool ValidatePerformanceMetrics();

    // Emergency validation after bridge failures
    UFUNCTION(BlueprintCallable, Category = "QA Emergency")
    void EmergencyValidationAfterBridgeFailure();

    UFUNCTION(BlueprintCallable, Category = "QA Emergency")
    void GenerateCriticalIncidentReport(const FString& IncidentType, const FString& Details);

    // Validation results
    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bGameSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bVFXSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bCharacterSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bWorldSystemsValid;

    UPROPERTY(BlueprintReadOnly, Category = "QA Results")
    bool bPerformanceValid;

    // Critical thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxActorCount = 15000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    float MaxFrameTime = 33.33f; // 30 FPS threshold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "QA Thresholds")
    int32 MaxMemoryUsageMB = 4096;

    // Validation history
    UPROPERTY(BlueprintReadOnly, Category = "QA History")
    TArray<FString> ValidationHistory;

    UPROPERTY(BlueprintReadOnly, Category = "QA History")
    TArray<FString> CriticalIncidents;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Internal validation helpers
    bool ValidateActorCount();
    bool ValidateMemoryUsage();
    bool ValidateFrameRate();
    bool ValidateAssetIntegrity();
    
    // Incident reporting
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    void ReportCriticalFailure(const FString& SystemName, const FString& FailureReason);

    // Validation state
    float LastValidationTime;
    int32 ValidationRunCount;
    bool bEmergencyMode;
};

/**
 * QA Validation Result Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_ValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString TestName;

    UPROPERTY(BlueprintReadOnly)
    bool bPassed;

    UPROPERTY(BlueprintReadOnly)
    FString Details;

    UPROPERTY(BlueprintReadOnly)
    float ExecutionTime;

    UPROPERTY(BlueprintReadOnly)
    FDateTime Timestamp;

    FQA_ValidationResult()
    {
        TestName = TEXT("");
        bPassed = false;
        Details = TEXT("");
        ExecutionTime = 0.0f;
        Timestamp = FDateTime::Now();
    }
};

/**
 * QA Critical Incident Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQA_CriticalIncident
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FString IncidentType;

    UPROPERTY(BlueprintReadOnly)
    FString SystemAffected;

    UPROPERTY(BlueprintReadOnly)
    FString Description;

    UPROPERTY(BlueprintReadOnly)
    int32 SeverityLevel; // 1-5, 5 being critical

    UPROPERTY(BlueprintReadOnly)
    FDateTime OccurrenceTime;

    UPROPERTY(BlueprintReadOnly)
    bool bResolved;

    FQA_CriticalIncident()
    {
        IncidentType = TEXT("");
        SystemAffected = TEXT("");
        Description = TEXT("");
        SeverityLevel = 1;
        OccurrenceTime = FDateTime::Now();
        bResolved = false;
    }
};