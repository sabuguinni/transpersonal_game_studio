#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationManager.h"
#include "CrowdGroupActor.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// ACrowdGroupActor
// Placed in the level to represent a crowd group's home territory.
// Reads from UCrowdSimulationManager and visualises group state in-editor.
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACrowdGroupActor : public AActor
{
    GENERATED_BODY()

public:
    ACrowdGroupActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Configuration ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    FName GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowd_GroupType GroupType = ECrowd_GroupType::HerbivoreHerd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 AgentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float WanderRadius = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float MovementSpeed = 280.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    float FleeThreshold = 0.5f;

    // ── Runtime State (read-only) ────────────────────────────────────────────

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_BehaviorState CurrentBehavior = ECrowd_BehaviorState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    float CurrentAlertLevel = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    bool bIsGroupAlerting = false;

    UPROPERTY(BlueprintReadOnly, Category = "Crowd")
    ECrowd_LODLevel CurrentLOD = ECrowd_LODLevel::LOD_Full;

    // ── Blueprint Events ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd")
    void OnGroupAlertTriggered(float AlertLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd")
    void OnGroupFleeing(FVector FleeDestination);

    UFUNCTION(BlueprintImplementableEvent, Category = "Crowd")
    void OnGroupCalmed();

    // ── Callable Functions ───────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void TriggerAlert(float AlertStrength, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Crowd")
    void RegisterWithCrowdManager();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FVector GetWanderTarget() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Crowd")
    FVector GetFleeDestination(FVector ThreatLocation) const;

private:
    UPROPERTY()
    UCrowdSimulationManager* CrowdManager;

    bool bWasAlerting = false;

    void SyncStateFromManager();
};
