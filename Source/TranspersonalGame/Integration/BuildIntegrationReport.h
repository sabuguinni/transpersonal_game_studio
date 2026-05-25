#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "BuildIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown         UMETA(DisplayName = "Unknown"),
    Success         UMETA(DisplayName = "Success"),
    Warning         UMETA(DisplayName = "Warning"),
    Failed          UMETA(DisplayName = "Failed"),
    Critical        UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString StatusMessage;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float PerformanceScore;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        StatusMessage = TEXT("");
        ActorCount = 0;
        PerformanceScore = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ReportTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ExpectedClassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallPerformanceScore;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalIssues;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> Warnings;

    FBuild_IntegrationReport()
    {
        ReportTimestamp = FDateTime::Now();
        OverallStatus = EBuild_IntegrationStatus::Unknown;
        TotalActorCount = 0;
        LoadedClassCount = 0;
        ExpectedClassCount = 0;
        OverallPerformanceScore = 0.0f;
    }
};

/**
 * Build Integration Report Manager
 * Handles validation and reporting of cross-system integration status
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationReportManager : public UObject
{
    GENERATED_BODY()

public:
    UBuildIntegrationReportManager();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus ValidateSystemIntegration(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateCoreClasses();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float CalculatePerformanceScore();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetCriticalIssues();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CoreClassPaths;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, EBuild_IntegrationStatus> SystemStatusCache;

private:
    void InitializeCoreClassPaths();
    FBuild_SystemStatus ValidateWorldGeneration();
    FBuild_SystemStatus ValidateCharacterSystems();
    FBuild_SystemStatus ValidateDinosaurSystems();
    FBuild_SystemStatus ValidateVFXSystems();
    FBuild_SystemStatus ValidateAudioSystems();
};