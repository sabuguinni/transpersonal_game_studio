#pragma once

#include "CoreMinimal.h"
#include "CrowdAgentTypes.generated.h"

// ============================================================
// Agent #13 — Crowd & Traffic Simulation
// CrowdAgentTypes.h — Shared types for crowd simulation
// Prehistoric survival game — Mass AI agent definitions
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_AgentSpecies : uint8
{
    HumanPrimitive      UMETA(DisplayName = "Human Primitive"),
    HerbivoreDino       UMETA(DisplayName = "Herbivore Dinosaur"),
    CarnivoreDino       UMETA(DisplayName = "Carnivore Dinosaur"),
    SmallCreature       UMETA(DisplayName = "Small Creature"),
    BirdFlocking        UMETA(DisplayName = "Bird Flocking"),
    FishSchool          UMETA(DisplayName = "Fish School")
};

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Wandering           UMETA(DisplayName = "Wandering"),
    Grazing             UMETA(DisplayName = "Grazing"),
    Fleeing             UMETA(DisplayName = "Fleeing"),
    Stampeding          UMETA(DisplayName = "Stampeding"),
    Hunting             UMETA(DisplayName = "Hunting"),
    Resting             UMETA(DisplayName = "Resting"),
    Migrating           UMETA(DisplayName = "Migrating"),
    Drinking            UMETA(DisplayName = "Drinking at Water Source"),
    Dead                UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_FleeReason : uint8
{
    None                UMETA(DisplayName = "None"),
    PredatorNearby      UMETA(DisplayName = "Predator Nearby"),
    PlayerNearby        UMETA(DisplayName = "Player Nearby"),
    LoudNoise           UMETA(DisplayName = "Loud Noise"),
    Fire                UMETA(DisplayName = "Fire"),
    Stampede            UMETA(DisplayName = "Stampede Contagion"),
    Injury              UMETA(DisplayName = "Injury")
};

UENUM(BlueprintType)
enum class ECrowd_LODLevel : uint8
{
    Full                UMETA(DisplayName = "Full Simulation (0-50m)"),
    Reduced             UMETA(DisplayName = "Reduced (50-200m)"),
    Minimal             UMETA(DisplayName = "Minimal (200-500m)"),
    Ghost               UMETA(DisplayName = "Ghost/Invisible (500m+)")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentSpecies Species = ECrowd_AgentSpecies::HumanPrimitive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float WalkSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float RunSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float FleeSpeed = 550.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float DetectionRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float FleeRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float HerdRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 MaxHerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float StampedeContagionRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bCanStampede = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bFlockingBehavior = false;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    ECrowd_AgentState CurrentState = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    ECrowd_FleeReason FleeReason = ECrowd_FleeReason::None;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    ECrowd_LODLevel LODLevel = ECrowd_LODLevel::Full;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    FVector CurrentTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    float StateTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    float Health = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    bool bIsHerdLeader = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Runtime")
    float LastStampedeSignalTime = -999.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    int32 HerdID = -1;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    ECrowd_AgentSpecies Species = ECrowd_AgentSpecies::HerbivoreDino;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    FVector HerdCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    int32 MemberCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    bool bIsStampeding = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Herd")
    float StampedeIntensity = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Origin = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Direction = FVector::ForwardVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Radius = 2000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Intensity = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_FleeReason Cause = ECrowd_FleeReason::PredatorNearby;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Timestamp = 0.0f;
};
