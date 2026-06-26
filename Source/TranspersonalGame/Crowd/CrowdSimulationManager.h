#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ─── Enums ────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_GroupType : uint8
{
    HerbivoreHerd     UMETA(DisplayName = "Herbivore Herd"),
    PredatorPack      UMETA(DisplayName = "Predator Pack"),
    ScavengerFlock    UMETA(DisplayName = "Scavenger Flock"),
    MigrationHerd     UMETA(DisplayName = "Migration Herd"),
    SolitaryAnimal    UMETA(DisplayName = "Solitary Animal"),
};

UENUM(BlueprintType)
enum class ECrowd_GroupState : uint8
{
    Idle      UMETA(DisplayName = "Idle"),
    Moving    UMETA(DisplayName = "Moving"),
    Fleeing   UMETA(DisplayName = "Fleeing"),
    Alert     UMETA(DisplayName = "Alert"),
    Grazing   UMETA(DisplayName = "Grazing"),
    Hunting   UMETA(DisplayName = "Hunting"),
};

// ─── Structs ──────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_GroupData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 GroupID = -1;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupType = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupState CurrentState = ECrowd_GroupState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float AlertLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeThreshold = 0.4f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 1000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 250.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = true;
};

// ─── UCrowdSimulationManager ──────────────────────────────────────────────────

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Group Management ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 RegisterCrowdGroup(ECrowd_GroupType GroupType, FVector SpawnCenter, int32 AgentCount);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    bool UnregisterCrowdGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetGroupTarget(int32 GroupID, FVector TargetLocation);

    // ── Alert & Flee ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void AlertGroupsInRadius(FVector AlertOrigin, float Radius, float AlertIntensity);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerGroupFlee(int32 GroupID, FVector ThreatLocation);

    // ── Query API ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveGroupCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetTotalAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FCrowd_GroupData GetGroupData(int32 GroupID) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<int32> GetGroupsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    ECrowd_GroupState GetGroupState(int32 GroupID) const;

    // ── Config ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float UpdateIntervalSeconds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FlockingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bDebugDraw;

private:
    void TickCrowdUpdate();
    void UpdateGroupBehavior(FCrowd_GroupData& Group, float DeltaTime);
    void DecayAlertLevels();

    TMap<int32, FCrowd_GroupData> ActiveGroups;
    int32 CurrentAgentCount = 0;
    int32 NextGroupID = 1;
    FTimerHandle UpdateTimerHandle;
};
