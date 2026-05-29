#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "UObject/NoExportTypes.h"
#include "BuildIntegrationReport.generated.h"

/**
 * Build Integration Report - Tracks compilation and system health
 * Agent #19 - Integration & Build Agent
 */

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Loading     UMETA(DisplayName = "Loading"),
    Operational UMETA(DisplayName = "Operational"),
    Failed      UMETA(DisplayName = "Failed"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemHealth
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System Health")
    float LoadTime;

    FBuild_SystemHealth()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        LastError = TEXT("");
        LoadTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 OperationalSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 FailedSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float OverallHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidation;

    FBuild_IntegrationMetrics()
    {
        TotalSystems = 0;
        OperationalSystems = 0;
        FailedSystems = 0;
        TotalActors = 0;
        OverallHealth = 0.0f;
        LastValidation = FDateTime::Now();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuildIntegrationReport : public UObject
{
    GENERATED_BODY()

public:
    UBuildIntegrationReport();

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    TArray<FBuild_SystemHealth> SystemHealthReports;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    FBuild_IntegrationMetrics IntegrationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    bool bBuildSuccessful;

    UPROPERTY(BlueprintReadOnly, Category = "Build Report")
    FString BuildVersion;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void GenerateHealthReport();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    float CalculateOverallHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool IsSystemOperational(const FString& SystemName) const;

private:
    void CheckCoreSystemHealth();
    void ValidateActorSpawning();
    void CheckBinaryIntegrity();
};

#include "BuildIntegrationReport.generated.h"