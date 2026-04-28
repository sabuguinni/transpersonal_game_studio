#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "CombatAITypes.h"
#include "CombatAIManager.generated.h"

class UCombatAIController;
class UBehaviorTreeComponent;

/**
 * Central manager for all combat AI coordination and tactical decision making
 * Handles pack behavior, threat assessment, and coordinated attacks
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCombatAIManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Register an AI controller with the combat manager */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatAI(UCombatAIController* AIController);

    /** Unregister an AI controller from the combat manager */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatAI(UCombatAIController* AIController);

    /** Get all registered combat AI controllers */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<UCombatAIController*> GetAllCombatAI() const;

    /** Find combat AI controllers within range of a position */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<UCombatAIController*> FindCombatAIInRange(const FVector& Position, float Range) const;

    /** Assess threat level for a target */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_ThreatLevel AssessThreatLevel(AActor* Target, APawn* Observer) const;

    /** Create or update pack coordination for a group of AI */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePack(const TArray<UCombatAIController*>& PackMembers, UCombatAIController* PackLeader);

    /** Execute coordinated attack on target */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteCoordinatedAttack(const TArray<UCombatAIController*>& Attackers, AActor* Target, ECombat_AttackPattern Pattern);

    /** Update pack formation */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdatePackFormation(const TArray<UCombatAIController*>& PackMembers, ECombat_Formation Formation, const FVector& CenterPoint);

    /** Calculate optimal attack pattern for given situation */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombat_AttackPattern CalculateOptimalAttackPattern(const TArray<UCombatAIController*>& Attackers, AActor* Target) const;

    /** Get pack coordination data for a specific AI */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FCombat_PackCoordination GetPackCoordination(UCombatAIController* AIController) const;

    /** Update global combat state (called by game mode) */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState(float DeltaTime);

    /** Emergency retreat signal for all AI in area */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void TriggerAreaRetreat(const FVector& DangerZone, float Radius, ECombat_ThreatLevel ThreatLevel);

protected:
    /** All registered combat AI controllers */
    UPROPERTY()
    TArray<UCombatAIController*> RegisteredAI;

    /** Pack coordination data mapped by pack leader */
    UPROPERTY()
    TMap<UCombatAIController*, FCombat_PackCoordination> PackCoordinationMap;

    /** Active coordinated attacks */
    UPROPERTY()
    TArray<FCombat_AttackData> ActiveAttacks;

    /** Global combat parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    FCombat_TacticalParams GlobalCombatParams;

    /** Maximum pack size for coordination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    int32 MaxPackSize;

    /** Combat update frequency (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Settings")
    float CombatUpdateFrequency;

    /** Last combat update time */
    float LastUpdateTime;

private:
    /** Calculate formation positions for pack members */
    TArray<FVector> CalculateFormationPositions(ECombat_Formation Formation, const FVector& CenterPoint, int32 MemberCount, float Spacing = 300.0f) const;

    /** Evaluate pack effectiveness */
    float EvaluatePackEffectiveness(const TArray<UCombatAIController*>& PackMembers) const;

    /** Find optimal pack leader from group */
    UCombatAIController* FindOptimalPackLeader(const TArray<UCombatAIController*>& Candidates) const;

    /** Update pack morale based on combat events */
    void UpdatePackMorale(UCombatAIController* PackLeader, float MoraleChange);

    /** Clean up invalid references */
    void CleanupInvalidReferences();

public:
    /** Blueprint event for pack coordination changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI Events")
    void OnPackCoordinationChanged(const TArray<UCombatAIController*>& PackMembers, ECombat_Formation NewFormation);

    /** Blueprint event for coordinated attack initiated */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI Events")
    void OnCoordinatedAttackInitiated(const TArray<UCombatAIController*>& Attackers, AActor* Target, ECombat_AttackPattern Pattern);

    /** Blueprint event for threat level changed */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI Events")
    void OnThreatLevelChanged(AActor* Target, ECombat_ThreatLevel OldLevel, ECombat_ThreatLevel NewLevel);

    /** Blueprint event for pack morale changed */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat AI Events")
    void OnPackMoraleChanged(UCombatAIController* PackLeader, float NewMorale, float MoraleChange);
};