// CrowdStampedeController.h
// Agent #13 — Crowd & Traffic Simulation
// Controls stampede events: triggers, direction, panic propagation, LOD management
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

UENUM(BlueprintType)
enum class ECrowd_StampedeTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    Predator        UMETA(DisplayName = "Predator Detected"),
    Explosion       UMETA(DisplayName = "Explosion / Loud Noise"),
    PlayerProximity UMETA(DisplayName = "Player Too Close"),
    Fire            UMETA(DisplayName = "Fire Detected"),
    AlphaSignal     UMETA(DisplayName = "Alpha Herd Signal")
};

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Alert       UMETA(DisplayName = "Alert"),
    Panic       UMETA(DisplayName = "Panic"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Settled     UMETA(DisplayName = "Settled")
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    AActor* AgentActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Speed = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsLeader = false;
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicPropagationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicPropagationSpeed = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MaxStampedeSpeed = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeDuration = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 MinAgentsForStampede = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bPlayerDangerIfInPath = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PlayerDamagePerSecond = 25.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FCrowd_StampedeConfig StampedeConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    ECrowd_StampedeState CurrentState;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    ECrowd_StampedeTrigger LastTrigger;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    TArray<FCrowd_StampedeAgent> RegisteredAgents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector StampedeDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    float StampedeTimer;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(ECrowd_StampedeTrigger Trigger, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void RegisterAgent(AActor* Agent, bool bIsLeader = false);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void UnregisterAgent(AActor* Agent);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void PropagateAlertToNearbyAgents(FVector EpicentreLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    bool IsPlayerInStampedePath(AActor* PlayerActor) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ForceSettle();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    int32 GetActiveAgentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    float GetAveragePanicLevel() const;

private:
    void UpdateStampedeState(float DeltaTime);
    void UpdateAgentMovement(float DeltaTime);
    void CheckPlayerCollision();
    FVector CalculateFleeDirection(FVector ThreatLocation, FVector AgentLocation) const;
    void TransitionToState(ECrowd_StampedeState NewState);

    float StateTimer;
    FVector ThreatOrigin;
    TWeakObjectPtr<AActor> CachedPlayerActor;
};
