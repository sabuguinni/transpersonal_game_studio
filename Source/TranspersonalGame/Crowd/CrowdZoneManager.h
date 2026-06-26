#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdZoneManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_ZoneType : uint8
{
    HerbivoreHerd   UMETA(DisplayName = "Herbivore Herd"),
    PredatorPack    UMETA(DisplayName = "Predator Pack"),
    MigrationRoute  UMETA(DisplayName = "Migration Route"),
    ApexTerritory   UMETA(DisplayName = "Apex Territory"),
    Scavenger       UMETA(DisplayName = "Scavenger Flock")
};

UENUM(BlueprintType)
enum class ECrowd_AlertState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Aggressive  UMETA(DisplayName = "Aggressive")
};

USTRUCT(BlueprintType)
struct FCrowd_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    ECrowd_ZoneType ZoneType = ECrowd_ZoneType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float ZoneRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    int32 MaxAgentsInZone = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float AlertRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float FleeThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    bool bIsMigrating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    FVector MigrationDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Zone")
    float MigrationSpeed = 150.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    ECrowd_AlertState AlertState = ECrowd_AlertState::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    float DistanceToPlayer = 9999.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    bool bIsAlpha = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Agent")
    FString GroupID;
};

/**
 * UCrowdZoneManager — World Subsystem managing prehistoric crowd simulation zones.
 * Handles herbivore herds, predator packs, migration routes, and apex territories.
 * Designed for up to 50,000 agents using Mass AI LOD chain.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdZoneManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdZoneManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Zone registration
    UFUNCTION(BlueprintCallable, Category = "Crowd|Zones")
    FString RegisterZone(const FVector& Center, const FCrowd_ZoneConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Zones")
    void UnregisterZone(const FString& ZoneID);

    // Alert system
    UFUNCTION(BlueprintCallable, Category = "Crowd|Alert")
    void TriggerAlertAtLocation(const FVector& AlertOrigin, float AlertRadius, ECrowd_AlertState NewState);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Alert")
    void PropagateAlertToNeighbours(const FString& ZoneID, ECrowd_AlertState State, int32 PropagationDepth = 2);

    // Migration
    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void StartMigration(const FString& ZoneID, const FVector& Destination, float Speed = 150.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void StopMigration(const FString& ZoneID);

    // Query
    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    TArray<FString> GetZonesInRadius(const FVector& Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    ECrowd_AlertState GetZoneAlertState(const FString& ZoneID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetTotalActiveAgents() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    FCrowd_ZoneConfig GetZoneConfig(const FString& ZoneID) const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void UpdateLODForPlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    void SetMaxSimulatedAgents(int32 MaxAgents);

    // Debug
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void PrintCrowdStatus() const;

private:
    // Zone registry: ZoneID -> (Config, Center, AlertState)
    TMap<FString, TPair<FCrowd_ZoneConfig, FVector>> ZoneRegistry;
    TMap<FString, ECrowd_AlertState> ZoneAlertStates;
    TMap<FString, int32> ZoneAgentCounts;

    int32 MaxSimulatedAgents = 500;
    int32 TotalActiveAgents = 0;
    int32 ZoneIDCounter = 0;

    // LOD thresholds (distance from player)
    static constexpr float LOD0_Distance = 2000.0f;  // Full simulation
    static constexpr float LOD1_Distance = 5000.0f;  // Reduced tick rate
    static constexpr float LOD2_Distance = 10000.0f; // Position-only update
    // Beyond LOD2: dormant (no simulation)

    FString GenerateZoneID(ECrowd_ZoneType Type);
    void UpdateZoneAgentCount(const FString& ZoneID);
    float GetDistanceBetweenZones(const FString& ZoneA, const FString& ZoneB) const;
};
