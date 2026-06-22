#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Herbivore_Herd      UMETA(DisplayName = "Herbivore Herd"),
    Predator_Pack       UMETA(DisplayName = "Predator Pack"),
    Scavenger_Solo      UMETA(DisplayName = "Scavenger Solo"),
    MigrationLeader     UMETA(DisplayName = "Migration Leader"),
    Sentry              UMETA(DisplayName = "Sentry")
};

UENUM(BlueprintType)
enum class ECrowd_DensityZone : uint8
{
    High    UMETA(DisplayName = "High Density"),
    Medium  UMETA(DisplayName = "Medium Density"),
    Low     UMETA(DisplayName = "Low Density"),
    Empty   UMETA(DisplayName = "Empty")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full        UMETA(DisplayName = "Full Simulation"),
    Reduced     UMETA(DisplayName = "Reduced Tick"),
    Minimal     UMETA(DisplayName = "Minimal Presence"),
    Culled      UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationCorridor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FString CorridorID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    ECrowd_AgentType AgentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float AgentSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 MaxAgentsInCorridor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsActive;

    FCrowd_MigrationCorridor()
        : CorridorID(TEXT(""))
        , AgentType(ECrowd_AgentType::Herbivore_Herd)
        , AgentSpeedMultiplier(1.0f)
        , MaxAgentsInCorridor(50)
        , bIsActive(true)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_DensityZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Density")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Density")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Density")
    ECrowd_DensityZone DensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Density")
    int32 MaxConcurrentAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Density")
    float AgentRespawnInterval;

    FCrowd_DensityZoneData()
        : Center(FVector::ZeroVector)
        , Radius(500.0f)
        , DensityLevel(ECrowd_DensityZone::Medium)
        , MaxConcurrentAgents(20)
        , AgentRespawnInterval(5.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_LODConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float FullSimDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float ReducedTickDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float MinimalPresenceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float FullSimTickRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float ReducedTickRate;

    FCrowd_LODConfig()
        : FullSimDistance(2000.0f)
        , ReducedTickDistance(5000.0f)
        , MinimalPresenceDistance(10000.0f)
        , CullDistance(15000.0f)
        , FullSimTickRate(0.1f)
        , ReducedTickRate(0.5f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxTotalAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    FCrowd_LODConfig LODConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_MigrationCorridor> MigrationCorridors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    TArray<FCrowd_DensityZoneData> DensityZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnableMigration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bEnablePlayerAvoidance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PlayerAvoidanceRadius;

    // --- Runtime State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    int32 ActiveAgentCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    int32 CulledAgentCount;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|State", meta = (AllowPrivateAccess = "true"))
    float CurrentSimulationLoad;

    // --- Public Interface ---
    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void InitializeCrowdSystem();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void SetMigrationActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void TriggerHerdFlee(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Control")
    void TriggerHerdGather(FVector GatherPoint, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    int32 GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    ECrowd_DensityZone GetDensityAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Query")
    ECrowd_LODLevel GetLODForDistance(float DistanceFromPlayer) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    void LogCrowdStats() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TickAccumulator;
    float MigrationTimer;

    void UpdateLODLevels();
    void ProcessMigrationCorridors(float DeltaTime);
    void EnforcePlayerAvoidance();
};
