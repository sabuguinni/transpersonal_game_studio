#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "CycleIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_CycleReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString CycleID;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 AgentNumber;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    FString AgentName;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    float CycleTimestamp;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    bool bCycleSuccess;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 FilesCreated;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    int32 UE5CommandsExecuted;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    TArray<FString> CreatedFiles;

    UPROPERTY(BlueprintReadOnly, Category = "Cycle")
    TArray<FString> Issues;

    FBuild_CycleReport()
    {
        CycleID = TEXT("");
        AgentNumber = 0;
        AgentName = TEXT("");
        CycleTimestamp = 0.0f;
        bCycleSuccess = false;
        FilesCreated = 0;
        UE5CommandsExecuted = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bUE5Connected;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bProjectCompiles;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMinPlayableMapLoaded;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorsInLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 LoadedModules;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> ActiveSystems;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> FailedSystems;

    FBuild_IntegrationStatus()
    {
        bUE5Connected = false;
        bProjectCompiles = false;
        bMinPlayableMapLoaded = false;
        TotalActorsInLevel = 0;
        LoadedModules = 0;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCycleIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCycleIntegrationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Cycle management
    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void StartNewCycle(const FString& CycleID, int32 AgentNumber, const FString& AgentName);

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void RecordFileCreation(const FString& FilePath);

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void RecordUE5Command(const FString& CommandDescription);

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void RecordIssue(const FString& IssueDescription);

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    FBuild_CycleReport FinalizeCycle(bool bSuccess);

    // Integration status
    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    FBuild_IntegrationStatus GetCurrentIntegrationStatus();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool ValidateCurrentCycle();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    TArray<FBuild_CycleReport> GetRecentCycles(int32 Count = 5);

    // System health checks
    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool CheckUE5Connectivity();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool CheckProjectCompilation();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool CheckMinPlayableMapState();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void GenerateIntegrationReport();

    // Emergency functions
    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    void EmergencyReset();

    UFUNCTION(BlueprintCallable, Category = "Cycle Integration")
    bool AttemptSystemRecovery();

protected:
    void LogCycleProgress();
    void UpdateIntegrationStatus();
    bool ValidateSystemIntegrity();

private:
    UPROPERTY()
    FBuild_CycleReport CurrentCycle;

    UPROPERTY()
    FBuild_IntegrationStatus CurrentStatus;

    UPROPERTY()
    TArray<FBuild_CycleReport> CycleHistory;

    UPROPERTY()
    bool bCycleInProgress;

    UPROPERTY()
    float LastStatusUpdate;
};