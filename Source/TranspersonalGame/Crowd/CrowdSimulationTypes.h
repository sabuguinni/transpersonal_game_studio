#pragma once

#include "CoreMinimal.h"
#include "CrowdSimulationTypes.generated.h"

// ============================================================
// Crowd Simulation Types — Agent #13
// Prehistoric survival game — herd/crowd behavior types
// ============================================================

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing       UMETA(DisplayName = "Grazing"),
    Wandering     UMETA(DisplayName = "Wandering"),
    Alerted       UMETA(DisplayName = "Alerted"),
    Stampeding    UMETA(DisplayName = "Stampeding"),
    Fleeing       UMETA(DisplayName = "Fleeing"),
    Resting       UMETA(DisplayName = "Resting")
};

UENUM(BlueprintType)
enum class ECrowd_HerbivoreSpecies : uint8
{
    Triceratops       UMETA(DisplayName = "Triceratops"),
    Brachiosaurus     UMETA(DisplayName = "Brachiosaurus"),
    Parasaurolophus   UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus       UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus      UMETA(DisplayName = "Ankylosaurus")
};

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    AlphaLeader   UMETA(DisplayName = "Alpha Leader"),
    Scout         UMETA(DisplayName = "Scout"),
    Follower      UMETA(DisplayName = "Follower"),
    Juvenile      UMETA(DisplayName = "Juvenile"),
    Sentry        UMETA(DisplayName = "Sentry")
};

UENUM(BlueprintType)
enum class ECrowd_ThreatLevel : uint8
{
    None      UMETA(DisplayName = "None"),
    Low       UMETA(DisplayName = "Low"),
    Medium    UMETA(DisplayName = "Medium"),
    High      UMETA(DisplayName = "High"),
    Critical  UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdMember
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsAlpha = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerbivoreSpecies Species = ECrowd_HerbivoreSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentRole Role = ECrowd_AgentRole::Follower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentID = -1;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector StampedeTriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector StampedeDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StampedeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float StampedeDuration = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_ThreatLevel ThreatLevel = ECrowd_ThreatLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsActive = false;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerbivoreSpecies Species = ECrowd_HerbivoreSpecies::Triceratops;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 HerdSize = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlertRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeSpeed = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float GrazeSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WalkSpeed = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float CohesionWeight = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float SeparationWeight = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float AlignmentWeight = 0.5f;
};
