#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
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
    int32 ActorCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FString LastValidationTime;

    FBuild_SystemStatus()
    {
        SystemName = TEXT("Unknown");
        bIsOperational = false;
        ActorCount = 0;
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
    FVector CenterLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    int32 PopulationCount;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bMeetsMinimumRequirement;

    FBuild_BiomeStatus()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        PopulationCount = 0;
        bMeetsMinimumRequirement = false;
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
        BuildVersion = TEXT("PROD_CYCLE_AUTO_20260516_005");
        LastIntegrationTime = FDateTime::Now();
    }
};

/**
 * Final Integration Orchestrator - Manages the complete build integration process
 * Validates all systems, coordinates biome population, and ensures build stability
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

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    FBuild_IntegrationReport GenerateIntegrationReport();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateAllSystems();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    bool ValidateBiomePopulation();

    UFUNCTION(BlueprintCallable, Category = "Integration", CallInEditor = true)
    void OrchestrateFinalBuild();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_SystemStatus GetSystemStatus(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    FBuild_BiomeStatus GetBiomeStatus(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsSystemOperational(const FString& SystemName);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    int32 GetTotalActorCount();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    bool IsBuildStable();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FBuild_IntegrationReport CurrentReport;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_SystemStatus> SystemStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    TMap<FString, FBuild_BiomeStatus> BiomeStatusMap;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    bool bIntegrationComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Integration")
    FDateTime LastValidationTime;

private:
    void InitializeBiomeDefinitions();
    void ValidateSystemIntegrity();
    void ValidateWorldPopulation();
    void ValidateVFXSystems();
    void ValidateLightingSystems();
    void ValidateCharacterSystems();
    void ValidatePhysicsSystems();
    void ValidateAudioSystems();
    
    FBuild_SystemStatus CreateSystemStatus(const FString& SystemName, bool bOperational, int32 ActorCount);
    FBuild_BiomeStatus CreateBiomeStatus(const FString& BiomeName, const FVector& Location, int32 Population);
    
    bool CheckSystemHealth(const FString& SystemName);
    int32 CountActorsInBiome(const FVector& BiomeCenter, float Radius = 25000.0f);
    
    static constexpr int32 MINIMUM_BIOME_POPULATION = 500;
    static constexpr int32 MINIMUM_TOTAL_ACTORS = 2500;
};