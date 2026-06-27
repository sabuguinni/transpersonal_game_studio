// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric crowd simulation: herds, packs, tribes with LOD and panic behavior

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Herbivore   UMETA(DisplayName = "Herbivore"),
    Predator    UMETA(DisplayName = "Predator"),
    Human       UMETA(DisplayName = "Human"),
    Scavenger   UMETA(DisplayName = "Scavenger")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full        UMETA(DisplayName = "Full Simulation"),
    Simplified  UMETA(DisplayName = "Simplified"),
    Billboard   UMETA(DisplayName = "Billboard Only")
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting")
};

// ============================================================
// STRUCTS — must be at global scope (UE5 compilation rule)
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::Full;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector PanicSource = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsPanicking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Health = 100.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType HerdType = ECrowd_AgentType::Herbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector MigrationDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MigrationSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 CurrentSize = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsMigrating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsPanicking = false;
};

USTRUCT(BlueprintType)
struct FCrowd_LODZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float ZoneRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxSimultaneousAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODCloseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float LODMediumRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float PanicRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    bool bSimulationActive;

    // --- Runtime Data ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_HerdData> RegisteredHerds;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_LODZone> LODZones;

    // --- Blueprint-callable Functions ---
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetHerdCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetSimulationActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerHerdPanic(int32 HerdID, FVector PanicSource);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    TArray<FCrowd_AgentData> GetAgentsInRadius(FVector Center, float Radius) const;

private:
    float TimeSinceLastTick;

    void InitializeHerds();
    void UpdateAgentLOD();
    void UpdateHerdBehavior(float DeltaTime);
    void CheckPanicTriggers();
    void OnAgentLODChanged(const FCrowd_AgentData& Agent, ECrowd_LODLevel NewLOD);
};
