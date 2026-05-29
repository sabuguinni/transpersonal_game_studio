#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "BuildIntegrationReport.generated.h"

UENUM(BlueprintType)
enum class EBuild_IntegrationStatus : uint8
{
    Unknown = 0,
    Operational = 1,
    Warning = 2,
    Critical = 3,
    Failed = 4
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
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        Status = EBuild_IntegrationStatus::Unknown;
        ActorCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidation;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    EBuild_IntegrationStatus OverallStatus;

    FBuild_IntegrationData()
    {
        TotalActors = 0;
        LoadedClasses = 0;
        FailedClasses = 0;
        LastValidation = FDateTime::Now();
        OverallStatus = EBuild_IntegrationStatus::Unknown;
    }
};

/**
 * Build Integration Report - Tracks system health and cross-module compatibility
 * Used by Agent #19 to validate that all agent outputs integrate correctly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UBuildIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationData GetIntegrationData() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    EBuild_IntegrationStatus GetSystemStatus(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void AddSystemStatus(const FString& SystemName, EBuild_IntegrationStatus Status, int32 ActorCount, const FString& ErrorMessage = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOperational(const FString& SystemName) const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetFailedSystems() const;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ClearReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FString GenerateReportSummary() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationData IntegrationData;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCharacterSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateVFXSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateFoliageSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ValidateAudioSystem();

private:
    void UpdateOverallStatus();
    bool ValidateClassLoading(const FString& ClassName);
    int32 CountActorsOfType(const FString& ActorType);
};