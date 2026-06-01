#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "AI/Combat/Combat_Types.h"
#include "Combat_TacticalAI.generated.h"

class ACharacter;
class UBehaviorTreeComponent;
class UBlackboardComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTacticalStateChanged, ECombat_TacticalState, OldState, ECombat_TacticalState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCombatTargetChanged, AActor*, NewTarget);

/**
 * Combat_TacticalAI - Advanced tactical AI system for dinosaur combat behavior
 * Handles complex combat decisions, positioning, group tactics, and adaptive behavior
 * Integrates with NPC animation system for seamless combat animations
 */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_TacticalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_TacticalAI();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TACTICAL STATE MANAGEMENT ===
    
    /** Current tactical state of this AI */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical AI")
    ECombat_TacticalState CurrentTacticalState;
    
    /** Previous tactical state for transition logic */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical AI")
    ECombat_TacticalState PreviousTacticalState;
    
    /** Time spent in current tactical state */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tactical AI")
    float StateTimer;
    
    /** Minimum time to stay in current state before allowing transitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical AI")
    float MinStateTime;

    // === COMBAT TARGET MANAGEMENT ===
    
    /** Current primary combat target */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    AActor* PrimaryCombatTarget;
    
    /** Secondary targets for group combat scenarios */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Target")
    TArray<AActor*> SecondaryCombatTargets;
    
    /** Maximum number of targets to track simultaneously */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxTrackedTargets;
    
    /** Range for detecting potential combat targets */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Target", meta = (ClampMin = "100.0", ClampMax = "5000.0"))
    float CombatDetectionRange;

    // === TACTICAL BEHAVIOR PARAMETERS ===
    
    /** Species-specific tactical profile */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    ECombat_DinosaurSpecies SpeciesTacticalType;
    
    /** Aggression level - affects attack frequency and risk-taking */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel;
    
    /** Intelligence level - affects tactical complexity and adaptation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float IntelligenceLevel;
    
    /** Pack coordination strength for group tactics */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackCoordinationStrength;
    
    /** Territorial behavior strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialStrength;

    // === POSITIONING AND MOVEMENT ===
    
    /** Preferred combat distance from target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
    float PreferredCombatDistance;
    
    /** Maximum distance before disengaging from combat */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Positioning")
    float MaxCombatDistance;
    
    /** Current tactical position relative to target */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Positioning")
    FVector TacticalPosition;
    
    /** Desired movement direction for tactical positioning */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Positioning")
    FVector DesiredMovementDirection;

    // === GROUP TACTICS ===
    
    /** Pack members for coordinated tactics */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Group Tactics")
    TArray<UCombat_TacticalAI*> PackMembers;
    
    /** Whether this AI is the pack leader */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Group Tactics")
    bool bIsPackLeader;
    
    /** Pack leader reference for coordination */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Group Tactics")
    UCombat_TacticalAI* PackLeader;
    
    /** Group formation type for coordinated attacks */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group Tactics")
    ECombat_FormationType GroupFormation;

    // === ADAPTIVE BEHAVIOR ===
    
    /** Player behavior analysis for adaptation */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive Behavior")
    FCombat_PlayerBehaviorProfile PlayerBehaviorProfile;
    
    /** Combat effectiveness history for learning */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptive Behavior")
    TArray<FCombat_EncounterData> CombatHistory;
    
    /** Maximum combat encounters to remember */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Behavior", meta = (ClampMin = "5", ClampMax = "50"))
    int32 MaxCombatMemory;

    // === EVENTS ===
    
    /** Broadcast when tactical state changes */
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnTacticalStateChanged OnTacticalStateChanged;
    
    /** Broadcast when combat target changes */
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatTargetChanged OnCombatTargetChanged;

    // === PUBLIC INTERFACE ===
    
    /** Set new tactical state with validation */
    UFUNCTION(BlueprintCallable, Category = "Tactical AI")
    void SetTacticalState(ECombat_TacticalState NewState);
    
    /** Get current tactical effectiveness rating */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tactical AI")
    float GetTacticalEffectiveness() const;
    
    /** Set primary combat target */
    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    void SetPrimaryCombatTarget(AActor* NewTarget);
    
    /** Add secondary combat target */
    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    void AddSecondaryCombatTarget(AActor* Target);
    
    /** Remove combat target */
    UFUNCTION(BlueprintCallable, Category = "Combat Target")
    void RemoveCombatTarget(AActor* Target);
    
    /** Get best tactical position for current situation */
    UFUNCTION(BlueprintCallable, Category = "Combat Positioning")
    FVector CalculateOptimalTacticalPosition();
    
    /** Join pack for coordinated tactics */
    UFUNCTION(BlueprintCallable, Category = "Group Tactics")
    void JoinPack(UCombat_TacticalAI* Leader);
    
    /** Leave current pack */
    UFUNCTION(BlueprintCallable, Category = "Group Tactics")
    void LeavePack();
    
    /** Execute coordinated pack attack */
    UFUNCTION(BlueprintCallable, Category = "Group Tactics")
    void ExecutePackAttack();
    
    /** Analyze and adapt to player behavior */
    UFUNCTION(BlueprintCallable, Category = "Adaptive Behavior")
    void AnalyzePlayerBehavior(AActor* Player, float EncounterDuration, bool bPlayerVictorious);

protected:
    // === INTERNAL LOGIC ===
    
    /** Update tactical state based on current situation */
    void UpdateTacticalState(float DeltaTime);
    
    /** Scan for potential combat targets */
    void ScanForCombatTargets();
    
    /** Evaluate threat level of potential target */
    float EvaluateThreatLevel(AActor* Target) const;
    
    /** Calculate tactical positioning */
    void CalculateTacticalPositioning();
    
    /** Update pack coordination */
    void UpdatePackCoordination();
    
    /** Execute species-specific tactical behavior */
    void ExecuteSpeciesTactics();
    
    /** Update adaptive behavior learning */
    void UpdateAdaptiveLearning();
    
    /** Get reference to owner character */
    ACharacter* GetOwnerCharacter() const;
    
    /** Get reference to behavior tree component */
    UBehaviorTreeComponent* GetBehaviorTreeComponent() const;
    
    /** Get reference to blackboard component */
    UBlackboardComponent* GetBlackboardComponent() const;

private:
    // === INTERNAL STATE ===
    
    /** Cached owner character reference */
    UPROPERTY()
    ACharacter* OwnerCharacter;
    
    /** Cached behavior tree component */
    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComp;
    
    /** Cached blackboard component */
    UPROPERTY()
    UBlackboardComponent* BlackboardComp;
    
    /** Timer for target scanning */
    float TargetScanTimer;
    
    /** Target scan interval */
    float TargetScanInterval;
    
    /** Timer for pack coordination updates */
    float PackCoordinationTimer;
    
    /** Pack coordination update interval */
    float PackCoordinationInterval;
};