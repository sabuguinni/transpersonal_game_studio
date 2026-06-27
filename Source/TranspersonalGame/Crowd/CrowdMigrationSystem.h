// CrowdMigrationSystem.h
// Agent #13 — Crowd & Traffic Simulation
// Cycle: PROD_CYCLE_AUTO_20260627_009
// Herbivore migration paths, predator ambush zones, LOD crowd tiers

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdMigrationSystem.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Herbivore_Grazer     UMETA(DisplayName = "Herbivore Grazer"),
    Herbivore_Scout      UMETA(DisplayName = "Herbivore Scout"),
    Predator_AlphaBlocker UMETA(DisplayName = "Predator Alpha Blocker"),
    Predator_LeftFlanker  UMETA(DisplayName = "Predator Left Flanker"),
    Predator_RightFlanker UMETA(DisplayName = "Predator Right Flanker"),
    Predator_Territorial  UMETA(DisplayName = "Predator Territorial"),
};

UENUM(BlueprintType)
enum class ECrowd_LODTier : uint8
{
    LOD0_Individual  UMETA(DisplayName = "LOD0 Individual (<500u)"),
    LOD1_Group       UMETA(DisplayName = "LOD1 Group (500-2000u)"),
    LOD2_Billboard   UMETA(DisplayName = "LOD2 Billboard (>2000u)"),
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationWaypoint
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    FVector WorldPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 WaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float ArrivalRadius;

    FCrowd_MigrationWaypoint()
        : WorldPosition(FVector::ZeroVector)
        , WaypointIndex(0)
        , ArrivalRadius(200.0f)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_AgentState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_LODTier LODTier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 CurrentWaypointIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float MoveSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsAlarmed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsActive;

    FCrowd_AgentState()
        : Role(ECrowd_AgentRole::Herbivore_Grazer)
        , LODTier(ECrowd_LODTier::LOD0_Individual)
        , CurrentWaypointIndex(0)
        , MoveSpeed(200.0f)
        , bIsAlarmed(false)
        , bIsActive(true)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdMigrationSystem : public AActor
{
    GENERATED_BODY()

public:
    ACrowdMigrationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Migration corridor waypoints (West Forest → River → Eastern Plains)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FCrowd_MigrationWaypoint> MigrationPath;

    // All active crowd agents
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agents")
    TArray<FCrowd_AgentState> ActiveAgents;

    // LOD distances
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD0_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD1_Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD2_Distance;

    // Max simultaneous agents per LOD tier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    int32 MaxAgentsLOD0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    int32 MaxAgentsLOD1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Performance")
    int32 MaxAgentsLOD2;

    // Alarm radius — when predator/player enters, herd scatters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Behavior")
    float AlarmRadius;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void InitializeMigrationPath();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Migration")
    void AdvanceHerdAlongPath(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|LOD")
    ECrowd_LODTier CalculateLODTier(FVector AgentLocation, FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void TriggerAlarm(FVector AlarmSource, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Behavior")
    void ScatterHerd(FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Debug")
    int32 GetActiveAgentCount() const;
};
