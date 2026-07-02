// CrowdStampedeController.h — Agent #13 Crowd & Traffic Simulation
// Stampede trigger system: mass panic events for herbivore herds fleeing predators
// Transpersonal Game Studio — Prehistoric Survival Game
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Crowd/CrowdSimulationManager.h"
#include "CrowdStampedeController.generated.h"

// ─── Enums (global scope — RULE 1) ───────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_StampedeState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Alert       UMETA(DisplayName = "Alert"),
    Panic       UMETA(DisplayName = "Panic"),
    Stampeding  UMETA(DisplayName = "Stampeding"),
    Dispersing  UMETA(DisplayName = "Dispersing")
};

UENUM(BlueprintType)
enum class ECrowd_StampedeTrigger : uint8
{
    PredatorProximity   UMETA(DisplayName = "Predator Proximity"),
    LoudNoise           UMETA(DisplayName = "Loud Noise"),
    PlayerAttack        UMETA(DisplayName = "Player Attack"),
    FireOrSmoke         UMETA(DisplayName = "Fire or Smoke"),
    EarthquakeVibration UMETA(DisplayName = "Earthquake Vibration")
};

// ─── Structs (global scope — RULE 1) ─────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeTrigger TriggerType = ECrowd_StampedeTrigger::PredatorProximity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector TriggerLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicIntensity = 1.0f;  // 0.0 = mild alert, 1.0 = full stampede

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PropagationSpeed = 800.0f;  // cm/s — how fast panic spreads through herd

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float Duration = 30.0f;  // seconds before herd calms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector FleeDirection = FVector::ForwardVector;
};

USTRUCT(BlueprintType)
struct FCrowd_HerdAgent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    int32 AgentID = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    ECrowd_StampedeState State = ECrowd_StampedeState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float PanicLevel = 0.0f;  // 0.0 calm → 1.0 full panic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    float Mass = 500.0f;  // kg — affects collision damage to player

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herd")
    bool bIsLeader = false;
};

// ─── Main Class ───────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ACrowdStampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowdStampedeController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Configuration ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 MaxHerdSize = 40;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float HerdSpawnRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_AgentType HerdSpecies = ECrowd_AgentType::DinosaurHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeed = 1200.0f;  // cm/s at full panic

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float CalmSpeed = 200.0f;  // cm/s when grazing

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PlayerTrampleDamage = 35.0f;  // damage per hit from stampeding agent

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicPropagationRadius = 600.0f;  // cm — panic spreads to nearby agents

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stampede")
    ECrowd_StampedeState CurrentHerdState = ECrowd_StampedeState::Calm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stampede")
    int32 ActiveAgentCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stampede")
    float AveragePanicLevel = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Stampede")
    FVector CurrentFleeDirection = FVector::ZeroVector;

    // ── Blueprint Events ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(const FCrowd_StampedeEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void CalmHerd();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void SpawnHerd(FVector CenterLocation, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    float GetPlayerDangerLevel(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    bool IsPlayerInStampedePath(FVector PlayerLocation, float CheckRadius = 400.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    TArray<FCrowd_HerdAgent> GetNearbyAgents(FVector Location, float Radius) const;

    UFUNCTION(BlueprintNativeEvent, Category = "Crowd|Stampede")
    void OnStampedeStarted(FVector FleeDirection, int32 AgentCount);

    UFUNCTION(BlueprintNativeEvent, Category = "Crowd|Stampede")
    void OnStampedeEnded();

    UFUNCTION(BlueprintNativeEvent, Category = "Crowd|Stampede")
    void OnPlayerTrampled(float DamageDealt, FVector ImpactVelocity);

private:
    UPROPERTY()
    TArray<FCrowd_HerdAgent> HerdAgents;

    UPROPERTY()
    float StampedeTimer = 0.0f;

    UPROPERTY()
    float StampedeDuration = 0.0f;

    void UpdateAgentPositions(float DeltaTime);
    void PropagatePanic(int32 AgentIndex);
    void UpdateHerdState();
    FVector ComputeFleeDirection(FVector ThreatLocation) const;
    FVector ComputeSeparationForce(int32 AgentIndex) const;
    FVector ComputeAlignmentForce(int32 AgentIndex) const;
    FVector ComputeCohesionForce(int32 AgentIndex) const;
};
