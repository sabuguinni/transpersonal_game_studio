// CrowdSimulationManager.h
// Agent #13 — Crowd & Traffic Simulation
// Prehistoric survival game — tribal crowd and dinosaur herd simulation

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CrowdSimulationManager.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 compilation rule)
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Tribal_Forager      UMETA(DisplayName = "Tribal Forager"),
    Tribal_Hunter       UMETA(DisplayName = "Tribal Hunter"),
    Tribal_Worker       UMETA(DisplayName = "Tribal Worker"),
    Tribal_Guard        UMETA(DisplayName = "Tribal Guard"),
    Herbivore_Herd      UMETA(DisplayName = "Herbivore Herd Member"),
    Predator_Pack       UMETA(DisplayName = "Predator Pack Member"),
    Scavenger           UMETA(DisplayName = "Scavenger"),
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    LOD0_FullDetail     UMETA(DisplayName = "LOD0 Full Detail (0-500m)"),
    LOD1_Medium         UMETA(DisplayName = "LOD1 Medium (500-1500m)"),
    LOD2_Low            UMETA(DisplayName = "LOD2 Low (1500-3000m)"),
    LOD3_Culled         UMETA(DisplayName = "LOD3 Culled (>3000m)"),
};

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Foraging            UMETA(DisplayName = "Foraging"),
    Patrolling          UMETA(DisplayName = "Patrolling"),
    Fleeing             UMETA(DisplayName = "Fleeing"),
    Stampeding          UMETA(DisplayName = "Stampeding"),
    Resting             UMETA(DisplayName = "Resting"),
    Socializing         UMETA(DisplayName = "Socializing"),
};

// ============================================================
// STRUCTS — must be at global scope
// ============================================================

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    TWeakObjectPtr<AActor> AgentActor;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentType AgentType = ECrowd_AgentType::Tribal_Forager;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::LOD0_FullDetail;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState BehaviorState = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CurrentSpeed = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsInStampede = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StateTimer = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdGroup
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FName HerdName;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<TWeakObjectPtr<AActor>> Members;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsMigrating = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float FlockingRadius = 400.0f;
};

// ============================================================
// UCLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdSimulationManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCrowdSimulationManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Agent registration
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterCrowdAgent(AActor* Agent, ECrowd_AgentType AgentType);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void UnregisterCrowdAgent(AActor* Agent);

    // Stampede system
    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(const FVector& Origin, const FVector& Direction, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void StopStampede();

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    bool IsStampedeActive() const { return bStampedeActive; }

    // Herd management
    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void RegisterHerdGroup(const FCrowd_HerdGroup& HerdGroup);

    // Queries
    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetAgentCountByType(ECrowd_AgentType AgentType) const;

    // Simulation tick (called by GameMode or timer)
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TickCrowdSimulation(float DeltaTime);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD0Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD1Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|LOD")
    float LOD2Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float TickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed;

private:
    UPROPERTY()
    TArray<FCrowd_AgentData> ActiveAgents;

    UPROPERTY()
    TArray<FCrowd_HerdGroup> HerdGroups;

    bool bStampedeActive;
    FVector StampedeDirection;
    FVector StampedeOrigin;

    void UpdateAgentLOD(FCrowd_AgentData& AgentData, const FVector& PlayerLocation);
};
