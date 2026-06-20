#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdHerdManager.generated.h"

UENUM(BlueprintType)
enum class ECrowd_HerdBehavior : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Migrating   UMETA(DisplayName = "Migrating"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Resting     UMETA(DisplayName = "Resting")
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_HerdBehavior Behavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float Health;

    FCrowd_HerdAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Behavior(ECrowd_HerdBehavior::Grazing)
        , Health(100.f)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdHerdManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdHerdManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void InitializeHerd(int32 AgentCount, FVector CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void SetHerdBehavior(ECrowd_HerdBehavior NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerFleeFromLocation(FVector ThreatLocation, float ThreatRadius);

    UFUNCTION(BlueprintPure, Category = "Crowd")
    int32 GetAgentCount() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float AgentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    ECrowd_HerdBehavior CurrentBehavior;

private:
    TArray<FCrowd_HerdAgent> Agents;

    void StepBoidsSimulation(float DeltaTime);
    FVector ComputeSeparation(int32 AgentIndex) const;
    FVector ComputeAlignment(int32 AgentIndex) const;
    FVector ComputeCohesion(int32 AgentIndex) const;
};
