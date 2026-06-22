// NPCBehaviorComponent.h
// NPC Behavior Agent #11 — Transpersonal Game Studio
// Behavior component for prehistoric survivor NPCs

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorComponent.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Flee        UMETA(DisplayName = "Flee"),
    Forage      UMETA(DisplayName = "Forage"),
    Shelter     UMETA(DisplayName = "Shelter"),
    Alert       UMETA(DisplayName = "Alert"),
    Dead        UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FNPC_ThreatMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    AActor* ThreatActor = nullptr;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatLevel = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float LastSeenTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector LastKnownLocation = FVector::ZeroVector;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current behavior state
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior")
    ENPC_BehaviorState CurrentState;

    // Detection radius for threats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float ThreatDetectionRadius = 3000.0f;

    // Flee speed multiplier when threatened
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FleeSpeedMultiplier = 1.8f;

    // Patrol radius around home location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 1500.0f;

    // Memory of recent threats
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_ThreatMemory> ThreatMemory;

    // Home base location
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    FVector HomeLocation;

    // Is this NPC currently aware of the player?
    UPROPERTY(BlueprintReadOnly, Category = "NPC|Behavior")
    bool bAwareOfPlayer = false;

    // Transition to a new behavior state
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    // Register a new threat in memory
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, float ThreatLevel);

    // Forget old threats (called on timer)
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void DecayThreatMemory(float DeltaTime);

    // Get the highest current threat level
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetMaxThreatLevel() const;

    // Check if NPC should flee
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool ShouldFlee() const;

private:
    float TimeSinceLastPatrolUpdate = 0.0f;
    FVector CurrentPatrolTarget;
    bool bHasPatrolTarget = false;

    void UpdateIdleBehavior(float DeltaTime);
    void UpdatePatrolBehavior(float DeltaTime);
    void UpdateFleeBehavior(float DeltaTime);
    void UpdateForageBehavior(float DeltaTime);
    void ScanForThreats();
    FVector GetRandomPatrolPoint() const;
};
