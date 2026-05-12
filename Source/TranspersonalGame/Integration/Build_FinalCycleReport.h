#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalCycleReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_CycleStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    InProgress      UMETA(DisplayName = "In Progress"),
    Completed       UMETA(DisplayName = "Completed"),
    Failed          UMETA(DisplayName = "Failed"),
    Validated       UMETA(DisplayName = "Validated")
};

UENUM(BlueprintType)
enum class EBuild_SystemHealth : uint8
{
    Critical        UMETA(DisplayName = "Critical"),
    Warning         UMETA(DisplayName = "Warning"),
    Healthy         UMETA(DisplayName = "Healthy"),
    Optimal         UMETA(DisplayName = "Optimal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemValidationResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    EBuild_SystemHealth HealthStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bCompilationSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    bool bRuntimeFunctional;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation")
    FString ValidationNotes;

    FBuild_SystemValidationResult()
    {
        SystemName = TEXT("Unknown");
        HealthStatus = EBuild_SystemHealth::Critical;
        bCompilationSuccess = false;
        bRuntimeFunctional = false;
        ActorCount = 0;
        ValidationNotes = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleCompletionData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    EBuild_CycleStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    float ExecutionTimeSeconds;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 TotalAgentsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 SuccessfulAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 FailedAgents;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    TArray<FBuild_SystemValidationResult> SystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString FinalReport;

    FBuild_CycleCompletionData()
    {
        CycleID = TEXT("UNKNOWN");
        Status = EBuild_CycleStatus::Unknown;
        ExecutionTimeSeconds = 0.0f;
        TotalAgentsExecuted = 0;
        SuccessfulAgents = 0;
        FailedAgents = 0;
        FinalReport = TEXT("");
    }
};

/**
 * Final Cycle Report Generator - Integration Agent #19
 * Generates comprehensive reports for completed production cycles
 * Validates all systems and provides final delivery status
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalCycleReport : public UActorComponent
{
    GENERATED_BODY()

public:
    UBuild_FinalCycleReport();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FBuild_CycleCompletionData CurrentCycleData;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FBuild_SystemValidationResult> CriticalSystemResults;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    bool bBridgeOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    bool bAllSystemsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString BuildHealthSummary;

    // Core validation functions
    UFUNCTION(BlueprintCallable, Category = "Validation")
    void GenerateFinalCycleReport(const FString& CycleID);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAllCriticalSystems();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    FBuild_SystemValidationResult ValidateSystem(const FString& SystemName, const FString& ClassPath);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void ValidateBridgeConnection();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    void CountLevelActors();

    UFUNCTION(BlueprintCallable, Category = "Report")
    FString GenerateBuildHealthSummary();

    UFUNCTION(BlueprintCallable, Category = "Report")
    void ExportReportToFile(const FString& FilePath);

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnCycleReportGenerated(const FBuild_CycleCompletionData& CycleData);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnSystemValidationComplete(bool bAllSystemsHealthy);

private:
    void InitializeSystemValidation();
    void ValidateVFXSystems();
    void ValidateCoreSystems();
    void ValidateWorldSystems();
    void GenerateHealthMetrics();
};