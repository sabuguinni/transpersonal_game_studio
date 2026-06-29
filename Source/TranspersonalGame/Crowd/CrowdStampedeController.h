// CrowdStampedeController.h
// Agent #13 — Crowd & Traffic Simulation
// Stampede trigger, propagation, and escape logic for prehistoric herds

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdStampedeController.generated.h"

// ─── Enums ───────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ECrowd_StampedeTrigger : uint8
{
    None            UMETA(DisplayName = "None"),
    Predator        UMETA(DisplayName = "Predator Detected"),
    Thunder         UMETA(DisplayName = "Thunder / Loud Noise"),
    Earthquake      UMETA(DisplayName = "Earthquake"),
    Fire            UMETA(DisplayName = "Fire / Smoke"),
    PlayerProximity UMETA(DisplayName = "Player Too Close")
};

UENUM(BlueprintType)
enum class ECrowd_StampedePhase : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Panic       UMETA(DisplayName = "Panic Spreading"),
    Running     UMETA(DisplayName = "Full Stampede"),
    Dispersing  UMETA(DisplayName = "Dispersing"),
    Settled     UMETA(DisplayName = "Settled")
};

// ─── Structs ─────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FCrowd_StampedeAgent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Location;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector Velocity;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicLevel; // 0.0 calm → 1.0 full panic

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    bool bIsPanicking;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    int32 AgentIndex;

    FCrowd_StampedeAgent()
        : Location(FVector::ZeroVector)
        , Velocity(FVector::ZeroVector)
        , PanicLevel(0.f)
        , bIsPanicking(false)
        , AgentIndex(-1)
    {}
};

USTRUCT(BlueprintType)
struct FCrowd_StampedeEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    ECrowd_StampedeTrigger Trigger;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    FVector EpicentreLocation;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float PropagationRadius;

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float Intensity; // 0.0–1.0

    UPROPERTY(BlueprintReadWrite, Category = "Crowd|Stampede")
    float TimeElapsed;

    FCrowd_StampedeEvent()
        : Trigger(ECrowd_StampedeTrigger::None)
        , EpicentreLocation(FVector::ZeroVector)
        , PropagationRadius(2000.f)
        , Intensity(1.f)
        , TimeElapsed(0.f)
    {}
};

// ─── Class ───────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = "Crowd", meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCrowdStampedeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UCrowdStampedeController();

    // ── Configuration ──────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicPropagationSpeed; // units/sec panic spreads between agents

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PanicDecayRate; // how fast panic fades when trigger gone

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float StampedeSpeedMultiplier; // run speed during full stampede

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float MinPanicThreshold; // panic level that triggers running

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Stampede")
    float PlayerDangerRadius; // radius within which player is trampled

    // ── Runtime State ──────────────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    ECrowd_StampedePhase CurrentPhase;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    FCrowd_StampedeEvent ActiveEvent;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    TArray<FCrowd_StampedeAgent> Agents;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd|Stampede")
    int32 PanickingAgentCount;

    // ── Public API ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void TriggerStampede(ECrowd_StampedeTrigger Trigger, FVector Epicentre, float Intensity = 1.f);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void RegisterAgent(FVector Location, int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ClearAllAgents();

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    bool IsPlayerInDanger(FVector PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    float GetHerdPanicLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    ECrowd_StampedePhase GetCurrentPhase() const { return CurrentPhase; }

    UFUNCTION(BlueprintCallable, Category = "Crowd|Stampede")
    void ForceSettle();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

private:
    void TickPanicPropagation(float DeltaTime);
    void TickPhaseTransitions();
    void TickAgentMovement(float DeltaTime);
    FVector ComputeFleeDirection(const FCrowd_StampedeAgent& Agent) const;
};
