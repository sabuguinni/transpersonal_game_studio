#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Build_FinalIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastError;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        ActorCount = 0;
        LastError = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    float MemoryUsagePercent;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildStable;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        MemoryUsagePercent = 0.0f;
        bBuildStable = false;
        LastValidationTime = FDateTime::Now();
    }
};

/**
 * Final Integration Manager - Orchestrates all game systems and validates build stability
 * Agent #19 responsibility: Ensure all 18 agent outputs work together seamlessly
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBuild_FinalIntegrationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UBuild_FinalIntegrationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core integration functions
    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_IntegrationReport ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool ValidateSystemIntegrity(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void ForceSystemReinitialization();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetCriticalErrors();

    // System monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    float GetMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsBuildStable();

    // Emergency functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void EmergencyCleanup();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void ValidateAndReport();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport LastReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FString> CriticalErrors;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bSystemsInitialized;

private:
    // Internal validation functions
    FBuild_SystemStatus ValidateWorldGeneration();
    FBuild_SystemStatus ValidateCharacterSystems();
    FBuild_SystemStatus ValidatePhysicsSystems();
    FBuild_SystemStatus ValidateAISystems();
    FBuild_SystemStatus ValidateAudioSystems();
    FBuild_SystemStatus ValidateVFXSystems();
    FBuild_SystemStatus ValidateQASystems();

    // Utility functions
    void LogSystemStatus(const FBuild_SystemStatus& Status);
    void UpdateCriticalErrors();
    bool CheckMemoryThresholds();
};