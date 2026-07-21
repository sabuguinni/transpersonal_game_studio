// CrowdHerdBehavior.h
// Agent #13 — Crowd & Traffic Simulation
// Herd behavior types for dinosaur crowd simulation

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CrowdHerdBehavior.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Resting     UMETA(DisplayName = "Resting"),
    Foraging    UMETA(DisplayName = "Foraging")
};

UENUM(BlueprintType)
enum class ECrowd_AgentRole : uint8
{
    Leader      UMETA(DisplayName = "Leader"),
    Follower    UMETA(DisplayName = "Follower"),
    Scout       UMETA(DisplayName = "Scout"),
    Juvenile    UMETA(DisplayName = "Juvenile"),
    Straggler   UMETA(DisplayName = "Straggler")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_AgentRole Role = ECrowd_AgentRole::Follower;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float FearLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float StaminaLevel = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsAlive = true;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    int32 HerdID = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FString SpeciesName = TEXT("Unknown");

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_HerdState CurrentState = ECrowd_HerdState::Grazing;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentCount = 0;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector CenterOfMass = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FVector MigrationTarget = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float CohesionRadius = 500.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float SeparationRadius = 150.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float AlignmentRadius = 300.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    float ThreatLevel = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCrowdHerdBehavior : public UObject
{
    GENERATED_BODY()

public:
    UCrowdHerdBehavior();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void InitializeHerd(int32 HerdID, const FString& Species, int32 NumAgents);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void UpdateHerdState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerFlee(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    void TriggerStampede(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Herd")
    FVector ComputeFlockingForce(const FCrowd_HerdAgent& Agent) const;

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    ECrowd_HerdState GetCurrentState() const { return HerdData.CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    int32 GetAgentCount() const { return HerdData.AgentCount; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Herd")
    FVector GetCenterOfMass() const { return HerdData.CenterOfMass; }

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    FCrowd_HerdData HerdData;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Herd")
    TArray<FCrowd_HerdAgent> Agents;

private:
    FVector ComputeCohesion(const FCrowd_HerdAgent& Agent) const;
    FVector ComputeSeparation(const FCrowd_HerdAgent& Agent) const;
    FVector ComputeAlignment(const FCrowd_HerdAgent& Agent) const;
    void UpdateCenterOfMass();
};
