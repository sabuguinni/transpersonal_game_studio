#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "Build_IntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_ValidationStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Pending     UMETA(DisplayName = "Pending"),
    InProgress  UMETA(DisplayName = "In Progress"),
    Passed      UMETA(DisplayName = "Passed"),
    Failed      UMETA(DisplayName = "Failed"),
    Blocked     UMETA(DisplayName = "Blocked")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_ModuleStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    FString ModuleName;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EBuild_ValidationStatus CompilationStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EBuild_ValidationStatus LinkingStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    EBuild_ValidationStatus RuntimeStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    int32 ErrorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Module")
    TArray<FString> ErrorMessages;

    FBuild_ModuleStatus()
    {
        ModuleName = TEXT("");
        CompilationStatus = EBuild_ValidationStatus::Unknown;
        LinkingStatus = EBuild_ValidationStatus::Unknown;
        RuntimeStatus = EBuild_ValidationStatus::Unknown;
        ClassCount = 0;
        ErrorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalModules;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 CompiledModules;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 FailedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float BuildTime;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    float MemoryUsage;

    FBuild_IntegrationMetrics()
    {
        TotalModules = 0;
        CompiledModules = 0;
        FailedModules = 0;
        TotalClasses = 0;
        LoadedClasses = 0;
        TotalActors = 0;
        BuildTime = 0.0f;
        MemoryUsage = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_IntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UBuild_IntegrationReport();

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FDateTime ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    EBuild_ValidationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FBuild_IntegrationMetrics Metrics;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FBuild_ModuleStatus> ModuleStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> Warnings;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FString> Recommendations;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void GenerateReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateModuleIntegrity(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_ModuleStatus GetModuleStatus(const FString& ModuleName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AddCriticalError(const FString& ErrorMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AddWarning(const FString& WarningMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AddRecommendation(const FString& RecommendationMessage);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemHealthy();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetOverallHealthScore();

private:
    void ValidateAllModules();
    void CalculateMetrics();
    void CheckSystemHealth();
    void GenerateRecommendations();
};