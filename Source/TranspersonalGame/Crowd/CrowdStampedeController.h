#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alarmed     UMETA(DisplayName = "Alarmed"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersed   UMETA(DisplayName = "Dispersed")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader;

    FCrowd_StampedeAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Speed(600.f)
        , FearLevel(0.f)
        , bIsLeader(false)
    {}
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(FVector PanicSource, float PanicRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SpawnHerd(int32 HerdSize, FVector CenterLocation, float SpreadRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void UpdateFlockingBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    FVector ComputeFlockingForce(const FCrowd_StampedeAgent& Agent, int32 AgentIndex);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SetStampedeState(ECrowd_StampedeState NewState);

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    ECrowd_StampedeState GetStampedeState() const { return CurrentState; }

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    int32 GetActiveAgentCount() const { return Agents.Num(); }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float FleeWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float NeighborRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float FearDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 MaxAgents;

private:
    UPROPERTY()
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY()
    ECrowd_StampedeState CurrentState;

    UPROPERTY()
    FVector PanicOrigin;

    UPROPERTY()
    TArray<AActor*> SpawnedMeshActors;

    void UpdateAgentMeshPositions();
    FVector ComputeSeparation(const FCrowd_StampedeAgent& Agent, int32 AgentIndex);
    FVector ComputeAlignment(const FCrowd_StampedeAgent& Agent, int32 AgentIndex);
    FVector ComputeCohesion(const FCrowd_StampedeAgent& Agent, int32 AgentIndex);
    FVector ComputeFlee(const FCrowd_StampedeAgent& Agent);
};
