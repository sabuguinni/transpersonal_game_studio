#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

// === Agent #13 Crowd & Traffic Simulation ===
// Crowd behavior enums and structs for prehistoric NPC simulation

UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Stampeding      UMETA(DisplayName = "Stampeding"),
    Migrating       UMETA(DisplayName = "Migrating"),
    Resting         UMETA(DisplayName = "Resting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Socializing     UMETA(DisplayName = "Socializing")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Panic       UMETA(DisplayName = "Panic")
};

UENUM(BlueprintType)
enum class ECrowd_HerdRole : uint8
{
    Alpha       UMETA(DisplayName = "Alpha"),
    Scout       UMETA(DisplayName = "Scout"),
    Follower    UMETA(DisplayName = "Follower"),
    Juvenile    UMETA(DisplayName = "Juvenile"),
    Straggler   UMETA(DisplayName = "Straggler")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdRole HerdRole = ECrowd_HerdRole::Follower;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector CurrentVelocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float FleeSpeed = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float WalkSpeed = 200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float SeparationRadius = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float CohesionRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float AlignmentRadius = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeTimer = 0.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FString SpeciesName = TEXT("Parasaurolophus");

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 MaxAgents = 30;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float TerritoryRadius = 5000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector HomeLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float MigrationDistance = 15000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bCanStampede = true;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeThreshold = 0.6f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float PanicRadius = 2000.0f;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 HerdID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection = FVector::ForwardVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeDuration = 15.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float StampedeSpeed = 1200.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = false;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ElapsedTime = 0.0f;
};
