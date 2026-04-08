#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "Engine/DataTable.h"
#include "CrowdSimulationSubsystem.generated.h"

class UMassEntityConfigAsset;
class AMassSpawner;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdGroupConfig : public FTableRowBase
{
    GENERATED_BODY()

    // Group identification
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FName GroupName;

    // Group composition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 MinGroupSize = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 MaxGroupSize = 12;

    // Movement behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float GroupCohesionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float SeparationRadius = 100.0f;

    // Spawning parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnDensityPerKm2 = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TArray<FName> PreferredBiomes;

    // Entity config reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass")
    TSoftObjectPtr<UMassEntityConfigAsset> EntityConfig;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMigrationRoute
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    FName RouteName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float SeasonalTrigger = 0.25f; // 0-1 representing season cycle

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    TArray<FName> ParticipatingGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    float MigrationSpeed = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Migration")
    bool bIsActive = false;
};

UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override { return true; }

    // Group management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void SpawnCrowdGroup(const FName& GroupType, const FVector& Location, int32 GroupSize = -1);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void DespawnCrowdGroup(const FMassEntityHandle& GroupLeader);

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateGroupDensity(const FName& BiomeName, float DensityMultiplier);

    // Migration system
    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StartMigration(const FName& RouteName);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void StopMigration(const FName& RouteName);

    UFUNCTION(BlueprintCallable, Category = "Migration")
    void UpdateSeasonalCycle(float SeasonProgress); // 0-1

    // Emergency responses
    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerPanicResponse(const FVector& ThreatLocation, float ThreatRadius, float ThreatIntensity);

    UFUNCTION(BlueprintCallable, Category = "Emergency")
    void TriggerStampedeResponse(const FVector& StampedeOrigin, const FVector& StampedeDirection);

    // Population management
    UFUNCTION(BlueprintCallable, Category = "Population")
    void SetGlobalPopulationScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Population")
    int32 GetActiveEntityCount() const;

    UFUNCTION(BlueprintCallable, Category = "Population")
    TArray<FMassEntityHandle> GetEntitiesInRadius(const FVector& Center, float Radius) const;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    UDataTable* CrowdGroupsDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    TArray<FMigrationRoute> MigrationRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    float MaxSimulationDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Config")
    int32 MaxActiveEntities = 50000;

    // Runtime data
    UPROPERTY()
    TMap<FName, TArray<FMassEntityHandle>> ActiveGroups;

    UPROPERTY()
    float CurrentSeasonProgress = 0.0f;

    UPROPERTY()
    float GlobalPopulationScale = 1.0f;

    // Subsystem references
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

private:
    void InitializeMassFramework();
    void UpdateMigrations(float DeltaTime);
    void UpdatePopulationLOD(float DeltaTime);
    void ProcessEmergencyResponses(float DeltaTime);
    
    FCrowdGroupConfig* GetGroupConfig(const FName& GroupType);
    FVector FindOptimalSpawnLocation(const FCrowdGroupConfig& Config, const FVector& PreferredLocation);
    
    // Emergency response tracking
    struct FEmergencyResponse
    {
        FVector Location;
        float Radius;
        float Intensity;
        float TimeRemaining;
        bool bIsStampede;
        FVector StampedeDirection;
    };
    
    TArray<FEmergencyResponse> ActiveEmergencyResponses;
    
    // Performance tracking
    float LastLODUpdateTime = 0.0f;
    const float LODUpdateInterval = 0.5f; // Update LOD twice per second
};