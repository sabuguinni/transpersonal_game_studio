// CrowdBehaviorTypes.h
// Agent #13 — Crowd & Traffic Simulation
// Shared enums and structs for crowd behavior system
#pragma once

#include "CoreMinimal.h"
#include "CrowdBehaviorTypes.generated.h"

UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Foraging    UMETA(DisplayName = "Foraging"),
    Socializing UMETA(DisplayName = "Socializing"),
    Sheltering  UMETA(DisplayName = "Sheltering"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None    UMETA(DisplayName = "None"),
    Low     UMETA(DisplayName = "Low"),
    Medium  UMETA(DisplayName = "Medium"),
    High    UMETA(DisplayName = "High"),
    Panic   UMETA(DisplayName = "Panic")
};

UENUM(BlueprintType)
enum class ECrowd_NpcRole : uint8
{
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Scout       UMETA(DisplayName = "Scout"),
    Guard       UMETA(DisplayName = "Guard"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Hunter      UMETA(DisplayName = "Hunter")
};

USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    int32 AgentIndex = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Idle;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_NpcRole Role = ECrowd_NpcRole::Gatherer;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Health = 100.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float Fear = 0.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_ThreatEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    FVector ThreatLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::None;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float ThreatRadius = 500.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd")
    float TimeStamp = 0.f;
};

USTRUCT(BlueprintType)
struct FCrowd_FlockingParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationRadius = 150.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentRadius = 400.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionRadius = 600.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float SeparationWeight = 1.5f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float AlignmentWeight = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float CohesionWeight = 0.8f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float MaxSpeed = 300.f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Flocking")
    float FleeSpeedMultiplier = 2.5f;
};
