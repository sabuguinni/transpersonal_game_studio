#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

UENUM(BlueprintType)
enum class EBuild_SystemStatus : uint8
{
    Unknown     UMETA(DisplayName = "Unknown"),
    Loading     UMETA(DisplayName = "Loading"),
    Ready       UMETA(DisplayName = "Ready"),
    Error       UMETA(DisplayName = "Error"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    EBuild_SystemStatus Status;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    FString LastError;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "System")
    int32 ActorCount;

    FBuild_SystemInfo()
    {
        SystemName = TEXT("");
        Status = EBuild_SystemStatus::Unknown;
        LastError = TEXT("");
        LastUpdateTime = 0.0f;
        ActorCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    TArray<FBuild_SystemInfo> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    int32 TotalActors;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    bool bAllSystemsReady;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Report")
    FDateTime LastIntegrationTime;

    FBuild_IntegrationReport()
    {
        TotalActors = 0;
        MemoryUsagePercent = 0.0f;
        bAllSystemsReady = false;
        BuildVersion = TEXT("PROD_CYCLE_AUTO_20260516_009");
        LastIntegrationTime = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Manages the complete build integration process
 * Validates all systems, monitors performance, and ensures build stability
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationOrchestrator : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationOrchestrator();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartIntegrationProcess();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void RefreshSystemStatuses();

    UFUNCTION(BlueprintPure, Category = "Integration")
    bool IsIntegrationComplete() const { return bIntegrationComplete; }

    UFUNCTION(BlueprintPure, Category = "Integration")
    float GetIntegrationProgress() const { return IntegrationProgress; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemInfo> SystemInfos;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float IntegrationProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastErrorMessage;

private:
    void ValidateCharacterSystem();
    void ValidateWorldGeneration();
    void ValidateFoliageSystem();
    void ValidateCrowdSimulation();
    void ValidateGameState();
    void ValidatePerformanceMetrics();
    
    FBuild_SystemInfo CreateSystemInfo(const FString& SystemName, EBuild_SystemStatus Status, const FString& Error = TEXT(""));
    void LogIntegrationStatus(const FString& Message);
    
    float StartTime;
    int32 ValidationSteps;
    int32 CompletedSteps;
};