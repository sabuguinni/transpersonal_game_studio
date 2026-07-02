#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Settled     UMETA(DisplayName = "Settled")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicLevel;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader;

    FCrowd_StampedeAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , Speed(600.f)
        , PanicLevel(0.f)
        , bIsLeader(false)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeTrigger
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicIntensity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FString TriggerSource;

    FCrowd_StampedeTrigger()
        : TriggerLocation(FVector::ZeroVector)
        , TriggerRadius(500.f)
        , PanicIntensity(1.0f)
        , TriggerSource(TEXT("Unknown"))
    {}
};

/**
 * UCrowd_StampedeController
 * Manages prehistoric herd stampede behavior — triggered by predators, thunder, or player actions.
 * Simulates up to 200 agents with flocking + panic propagation using lightweight tick-based simulation.
 */
UCLASS(ClassGroup = "Crowd", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowd_StampedeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowd_StampedeController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicPropagationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float AlignmentWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CohesionWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float SeparationWeight;

    // --- State ---

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    ECrowd_StampedeState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    float AveragePanicLevel;

    // --- API ---

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(const FCrowd_StampedeTrigger& Trigger);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void InitializeAgents(FVector CenterLocation, float SpawnRadius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SetStampedeDirection(FVector Direction);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetAgentsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void CalmHerd();

    UFUNCTION(BlueprintPure, Category = "Crowd|Stampede")
    bool IsStampeding() const { return CurrentState == ECrowd_StampedeState::Stampeding; }

private:
    void UpdateFlocking(float DeltaTime);
    void PropagatePanic(float DeltaTime);
    void UpdateState();
    FVector ComputeFlockingForce(int32 AgentIndex) const;

    FVector StampedeDirection;
    float StateTimer;
};
