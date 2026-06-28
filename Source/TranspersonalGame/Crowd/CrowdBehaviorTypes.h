#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// === CROWD BEHAVIOR TYPES — Agent #13 Crowd & Traffic Simulation ===
// Prehistoric survival game crowd simulation types

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing       UMETA(DisplayName = "Grazing"),
    Migrating     UMETA(DisplayName = "Migrating"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Stampeding    UMETA(DisplayName = "Stampeding"),
    Resting       UMETA(DisplayName = "Resting"),
    Drinking      UMETA(DisplayName = "Drinking"),
    Foraging      UMETA(DisplayName = "Foraging"),
    Alert         UMETA(DisplayName = "Alert"),
};

UENUM(BlueprintType)
enum class ECrowd_DinoSpecies : uint8
{
    Triceratops   UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus   UMETA(DisplayName = "Stegosaurus"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Gallimimus    UMETA(DisplayName = "Gallimimus"),
    Pachycephalosaurus UMETA(DisplayName = "Pachycephalosaurus"),
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None          UMETA(DisplayName = "None"),
    Low           UMETA(DisplayName = "Low"),
    Medium        UMETA(DisplayName = "Medium"),
    High          UMETA(DisplayName = "High"),
    Critical      UMETA(DisplayName = "Critical — Stampede"),
};

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Leader        UMETA(DisplayName = "Herd Leader"),
    Scout         UMETA(DisplayName = "Scout"),
    Juvenile      UMETA(DisplayName = "Juvenile"),
    Elder         UMETA(DisplayName = "Elder"),
    Sentinel      UMETA(DisplayName = "Sentinel"),
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_DinoSpecies Species = ECrowd_DinoSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdSize = 12;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float SpreadRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float MigrationSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float StampedeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatDetectionRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float WaterSourceRange = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bHasAlphaLeader = true;
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_AgentRole Role = ECrowd_AgentRole::Scout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    ECrowd_HerdState CurrentState = ECrowd_HerdState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float StressLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float HungerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float ThirstLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    bool bIsLeader = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float SeparationWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float CohesionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Agent")
    float AlignmentWeight = 1.2f;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection = FVector::ForwardVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float Duration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TramplingDamage = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::High;
};

USTRUCT(BlueprintType)
struct FCrowd_MigrationPath
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    TArray<FVector> Waypoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    int32 CurrentWaypointIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float WaypointAcceptanceRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    bool bIsLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Migration")
    float SeasonalCycleHours = 72.0f;
};
