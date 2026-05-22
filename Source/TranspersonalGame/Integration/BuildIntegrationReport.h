#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "BuildIntegrationReport.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    bool bIsCompiled;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    bool bIsLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Build Status")
    FString LastError;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("");
        bIsCompiled = false;
        bIsLoaded = false;
        ActorCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString CycleId;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime ReportTime;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalSourceFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 CompiledBinaries;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedClasses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 DinosaurActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 EnvironmentActors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMapSaved;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bAllSystemsOperational;

    FBuild_IntegrationReport()
    {
        CycleId = TEXT("");
        ReportTime = FDateTime::Now();
        TotalSourceFiles = 0;
        CompiledBinaries = 0;
        LoadedClasses = 0;
        TotalActors = 0;
        DinosaurActors = 0;
        EnvironmentActors = 0;
        bMapSaved = false;
        bAllSystemsOperational = false;
    }
};

/**
 * Build Integration Report Subsystem
 * Tracks compilation status, system health, and integration metrics
 */
UCLASS()
class TRANSPERSONALGAME_API UBuildIntegrationReport : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    FBuild_IntegrationReport GenerateIntegrationReport(const FString& CycleId);

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    bool ValidateSystemIntegration();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    TArray<FBuild_SystemStatus> GetSystemStatuses();

    UFUNCTION(BlueprintCallable, Category = "Build Integration")
    void LogIntegrationMetrics(const FBuild_IntegrationReport& Report);

protected:
    UPROPERTY()
    TArray<FBuild_IntegrationReport> ReportHistory;

    UPROPERTY()
    FBuild_IntegrationReport LastReport;

private:
    FBuild_SystemStatus CheckSystemStatus(const FString& SystemName, const FString& ClassPath);
    int32 CountActorsByType(const FString& ActorType);
    bool ValidateMapPersistence();
};