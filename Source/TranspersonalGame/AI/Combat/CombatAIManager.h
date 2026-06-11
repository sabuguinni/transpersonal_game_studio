#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "CombatAIManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_ThreatLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float ThreatValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.5f;

    FCombat_ThreatLevel()
    {
        ThreatValue = 0.0f;
        DetectionRadius = 1000.0f;
        AggressionLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_BehaviorState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_AIState CurrentState = ECombat_AIState::Patrol;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float StateTimer = 0.0f;

    FCombat_BehaviorState()
    {
        CurrentState = ECombat_AIState::Patrol;
        TargetActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        StateTimer = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIManager : public AActor
{
    GENERATED_BODY()

public:
    ACombatAIManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Combat AI Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<AActor*> RegisteredCombatants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    TArray<ATriggerBox*> CombatZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float GlobalThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    int32 MaxActiveCombatants = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    float CombatUpdateInterval = 0.5f;

    // Dinosaur Limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Management")
    int32 DinosaurHardCap = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Management")
    int32 CurrentDinosaurCount = 0;

private:
    float CombatUpdateTimer = 0.0f;
    TMap<AActor*, FCombat_BehaviorState> CombatantStates;
    TMap<AActor*, FCombat_ThreatLevel> ThreatLevels;

public:
    // Combat Management Functions
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatant(AActor* Combatant, const FCombat_ThreatLevel& ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatant(AActor* Combatant);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatBehaviors();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(AActor* Combatant, ECombat_AIState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AIState GetCombatState(AActor* Combatant) const;

    // Dinosaur Management Functions
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Management")
    void EnforceDinosaurLimit();

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Management")
    int32 CountDinosaurs() const;

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Management")
    bool CanSpawnDinosaur() const;

    // Combat Zone Management
    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void RegisterCombatZone(ATriggerBox* Zone);

    UFUNCTION(BlueprintCallable, Category = "Combat Zones")
    void CheckCombatZoneActivation();

    // Threat Assessment
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    float CalculateGlobalThreat() const;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    AActor* FindNearestThreat(AActor* FromActor, float SearchRadius = 2000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    TArray<AActor*> GetThreatsInRadius(const FVector& Location, float Radius) const;
};