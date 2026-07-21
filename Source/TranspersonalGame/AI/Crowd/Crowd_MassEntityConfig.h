#pragma once

#include "CoreMinimal.h"
#include "MassEntityTypes.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassRepresentationFragments.h"
#include "MassNavigationFragments.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Crowd_MassEntityConfig.generated.h"

// Crowd behavior states for Mass Entity system
UENUM(BlueprintType)
enum class ECrowd_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),
    Running     UMETA(DisplayName = "Running"),
    Gathering   UMETA(DisplayName = "Gathering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following")
};

// Crowd agent types for different primitive human behaviors
UENUM(BlueprintType)
enum class ECrowd_AgentType : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Child       UMETA(DisplayName = "Child"),
    Elder       UMETA(DisplayName = "Elder"),
    Guard       UMETA(DisplayName = "Guard")
};

// Mass Entity fragment for crowd behavior
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_BehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    ECrowd_BehaviorState CurrentState = ECrowd_BehaviorState::Idle;
    ECrowd_AgentType AgentType = ECrowd_AgentType::Gatherer;
    float StateTimer = 0.0f;
    float MaxStateTime = 5.0f;
    FVector TargetLocation = FVector::ZeroVector;
    bool bHasTarget = false;
    float MovementSpeed = 150.0f;
    float FleeRadius = 500.0f;
    int32 GroupID = 0;
};

// Mass Entity fragment for crowd social behavior
USTRUCT()
struct TRANSPERSONALGAME_API FCrowd_SocialFragment : public FMassFragment
{
    GENERATED_BODY()

    TArray<FMassEntityHandle> NearbyEntities;
    float SocialRadius = 200.0f;
    float AvoidanceRadius = 50.0f;
    float GroupCohesion = 0.5f;
    float SeparationWeight = 1.0f;
    float AlignmentWeight = 0.3f;
    bool bIsGroupLeader = false;
    FMassEntityHandle LeaderEntity;
};

// Configuration class for Mass Entity crowd simulation
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowd_MassEntityConfig : public UObject
{
    GENERATED_BODY()

public:
    UCrowd_MassEntityConfig();

    // Spawn configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxCrowdEntities = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnCenter = FVector::ZeroVector;

    // Behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float DefaultMovementSpeed = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float RunningSpeedMultiplier = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeInterval = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance = 800.0f;

    // LOD configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighLODDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumLODDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowLODDistance = 3000.0f;

    // Social behavior configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float GroupFormationRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 MaxGroupSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LeadershipProbability = 0.15f;

    // Performance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxProcessingEntitiesPerFrame = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableMultithreading = true;
};