#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "Build_FinalIntegrationOrchestrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_SystemStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIsOperational;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        ComponentCount = 0;
        LastValidationTime = TEXT("Never");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_BiomeStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BiomeName;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FVector BiomeCenter;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMeetsPopulationTarget;

    FBuild_BiomeStatus()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        ActorCount = 0;
        bMeetsPopulationTarget = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBuild_IntegrationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_SystemStatus> SystemStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TArray<FBuild_BiomeStatus> BiomeStatuses;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 TotalActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bBuildStable;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString BuildVersion;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastIntegrationTime;

    FBuild_IntegrationReport()
    {
        TotalActorCount = 0;
        bBuildStable = false;
        BuildVersion = TEXT("1.0.0");
        LastIntegrationTime = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Coordinates all game systems and validates build stability
 * Responsible for ensuring all 18 agent outputs integrate into a cohesive, playable experience
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

    // Core Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAssetPipeline();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateVFXSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateLightingAndAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateCharacterSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateDinosaurAI();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidatePhysicsAndCollision();

    // System Monitoring
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StartContinuousMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void StopContinuousMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    // Build Management
    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool CreateBuildSnapshot();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool RestoreFromSnapshot(const FString& SnapshotName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    TArray<FString> GetAvailableSnapshots();

protected:
    // System validation helpers
    FBuild_SystemStatus ValidateWorldGeneration();
    FBuild_SystemStatus ValidateEnvironmentArt();
    FBuild_SystemStatus ValidateArchitecture();
    FBuild_SystemStatus ValidateAnimation();
    FBuild_SystemStatus ValidateNPCBehavior();
    FBuild_SystemStatus ValidateCombatAI();
    FBuild_SystemStatus ValidateCrowdSimulation();
    FBuild_SystemStatus ValidateQuestSystem();
    FBuild_SystemStatus ValidateNarrative();
    FBuild_SystemStatus ValidateAudioSystems();

    // Biome validation helpers
    FBuild_BiomeStatus ValidateSavanaBiome();
    FBuild_BiomeStatus ValidatePantanoBiome();
    FBuild_BiomeStatus ValidateFlorestaBiome();
    FBuild_BiomeStatus ValidateDesertoBiome();
    FBuild_BiomeStatus ValidateMontanhaBiome();

    // Internal state
    UPROPERTY()
    TMap<FString, FBuild_SystemStatus> CachedSystemStatuses;

    UPROPERTY()
    TArray<FBuild_BiomeStatus> CachedBiomeStatuses;

    UPROPERTY()
    bool bMonitoringActive;

    UPROPERTY()
    FTimerHandle MonitoringTimerHandle;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    float MonitoringInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    int32 MinimumBiomePopulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration", meta = (AllowPrivateAccess = "true"))
    bool bAutoFixIssues;

private:
    void PerformMonitoringTick();
    void LogIntegrationStatus(const FBuild_IntegrationReport& Report);
    bool IsSystemCritical(const FString& SystemName);
};