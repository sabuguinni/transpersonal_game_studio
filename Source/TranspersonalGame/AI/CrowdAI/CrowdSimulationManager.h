#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.generated.h"

// Crowd agent behavioral states — prehistoric ecosystem
UENUM(BlueprintType)
enum class ECrowd_AgentState : uint8
{
    Grazing     UMETA(DisplayName = "Grazing"),
    Wandering   UMETA(DisplayName = "Wandering"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Drinking    UMETA(DisplayName = "Drinking"),
    Resting     UMETA(DisplayName = "Resting"),
    Dead        UMETA(DisplayName = "Dead")
};

// Species type for crowd agents
UENUM(BlueprintType)
enum class ECrowd_SpeciesType : uint8
{
    Herbivore_Large   UMETA(DisplayName = "Herbivore Large"),   // Brachiosaurus, Triceratops
    Herbivore_Medium  UMETA(DisplayName = "Herbivore Medium"),  // Parasaurolophus
    Herbivore_Small   UMETA(DisplayName = "Herbivore Small"),   // Gallimimus
    Omnivore          UMETA(DisplayName = "Omnivore"),          // Oviraptor
    Predator_Pack     UMETA(DisplayName = "Predator Pack"),     // Raptor
    Predator_Apex     UMETA(DisplayName = "Predator Apex")      // T-Rex
};

// Lightweight crowd agent data — used for Mass AI simulation
USTRUCT(BlueprintType)
struct FCrowd_AgentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector FleeDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_AgentState State = ECrowd_AgentState::Grazing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_SpeciesType Species = ECrowd_SpeciesType::Herbivore_Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearLevel = 0.0f;  // 0-1, drives flee threshold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MoveSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    bool bIsAlive = true;
};

// Predator threat stimulus — broadcast when apex predator enters Chasing state
USTRUCT(BlueprintType)
struct FCrowd_PredatorStimulus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FVector OriginLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FearIntensity = 1.0f;  // 1.0 = apex predator roar

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_SpeciesType PredatorSpecies = ECrowd_SpeciesType::Predator_Apex;
};

/**
 * ACrowdSimulationManager
 * Manages prehistoric ecosystem crowd simulation.
 * Handles herbivore herds, raptor packs, prey scatter, and fear propagation.
 * Designed to integrate with ATRexPawn::TriggerRoar() via OnPredatorRoar().
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdSimulationManager : public AActor
{
    GENERATED_BODY()

public:
    ACrowdSimulationManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxHerbivoreAgents = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    int32 MaxRaptorPackSize = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float HerdRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FearPropagationRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FleeSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Config")
    float FearDecayRate = 0.2f;  // Fear units lost per second when no predator present

    // --- Herd anchor positions (set in editor) ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herds")
    TArray<FVector> HerbivoreHerdAnchors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd|Herds")
    TArray<FVector> RaptorPackAnchors;

    // --- Runtime state ---

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentData> HerbivoreAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    TArray<FCrowd_AgentData> RaptorAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    bool bPredatorActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Crowd|Runtime")
    FVector ActivePredatorLocation = FVector::ZeroVector;

    // --- Public API ---

    // Called by ATRexPawn::TriggerRoar() — propagates fear to all nearby agents
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void OnPredatorRoar(const FCrowd_PredatorStimulus& Stimulus);

    // Called when TRex transitions to Chasing state
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void OnPredatorChasing(FVector PredatorLocation, ECrowd_SpeciesType PredatorType);

    // Called when TRex returns to Patrolling — fear subsides
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void OnPredatorRetreated();

    // Returns flee direction away from threat origin
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    FVector CalculateFleeDirection(FVector AgentLocation, FVector ThreatOrigin) const;

    // Returns number of agents currently in Fleeing state
    UFUNCTION(BlueprintCallable, Category = "Crowd")
    int32 GetFleeingAgentCount() const;

    // Debug: print crowd state to screen
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Crowd|Debug")
    void DebugPrintCrowdState() const;

private:
    void InitializeHerbivoreHerd();
    void InitializeRaptorPack();
    void UpdateHerbivoreAgents(float DeltaTime);
    void UpdateRaptorAgents(float DeltaTime);
    void PropagateFeaR(const FCrowd_PredatorStimulus& Stimulus);

    float TickAccumulator = 0.0f;
    static constexpr float CrowdTickInterval = 0.1f;  // Update crowd at 10Hz, not 60Hz
};
