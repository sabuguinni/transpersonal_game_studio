#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "CombatAISystem.h"
#include "CombatAIComponent.generated.h"

class UEnvironmentQueryContext;
class UEnvQuery;

/**
 * Core Combat AI Component
 * Manages tactical decision making, threat assessment, and combat behavior coordination
 * This component is the brain of combat AI - it processes threats, makes tactical decisions,
 * and coordinates with other systems to execute complex combat behaviors
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ===== CORE COMBAT CONFIGURATION =====
    
    /** Primary combat archetype - defines fundamental behavior patterns */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    ECombatArchetype CombatArchetype = ECombatArchetype::FlightResponse;

    /** Physical and tactical capabilities of this creature */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    FCombatCapabilities Capabilities;

    /** Behavior tree for combat decision making */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    UBehaviorTree* CombatBehaviorTree;

    /** EQS query for finding tactical positions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    UEnvQuery* TacticalPositionQuery;

    /** EQS query for finding escape routes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    UEnvQuery* EscapeRouteQuery;

    /** EQS query for finding ambush positions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    UEnvQuery* AmbushPositionQuery;

    // ===== CURRENT COMBAT STATE =====
    
    /** Current combat state */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState = ECombatState::Passive;

    /** Current tactical approach */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatTactic CurrentTactic = ECombatTactic::StandGround;

    /** Primary target for combat actions */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* PrimaryTarget;

    /** Secondary targets being tracked */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<AActor*> SecondaryTargets;

    /** Current threat assessments */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    TArray<FThreatAssessment> ThreatAssessments;

    /** Last known player location */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    FVector LastKnownPlayerLocation;

    /** Time since last player contact */
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float TimeSinceLastPlayerContact = 0.0f;

    // ===== COMBAT PARAMETERS =====
    
    /** How aggressive this creature is (0.0 = passive, 1.0 = extremely aggressive) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float AggressionLevel = 0.3f;

    /** How likely to flee when threatened (0.0 = never flee, 1.0 = always flee) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float FlightResponse = 0.7f;

    /** How well this creature works with others (pack coordination) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float CoordinationSkill = 0.5f;

    /** How territorial this creature is */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters", meta = (ClampMin = 0.0, ClampMax = 1.0))
    float TerritorialInstinct = 0.4f;

    /** Minimum distance to maintain from threats */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float PreferredCombatDistance = 300.0f;

    /** Maximum distance before losing interest in target */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float MaxEngagementDistance = 2000.0f;

    /** How long to remember threats after losing sight */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Parameters")
    float ThreatMemoryDuration = 30.0f;

    // ===== PACK BEHAVIOR =====
    
    /** Other combat AI components in the same pack/herd */
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    TArray<UCombatAIComponent*> PackMembers;

    /** Leader of this pack (nullptr if this is the leader or solo) */
    UPROPERTY(BlueprintReadOnly, Category = "Pack Behavior")
    UCombatAIComponent* PackLeader;

    /** Whether this creature is the pack leader */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    bool bIsPackLeader = false;

    /** Maximum pack size this creature will coordinate with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    int32 MaxPackSize = 6;

    /** Range for pack communication and coordination */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pack Behavior")
    float PackCoordinationRange = 1000.0f;

public:
    // ===== CORE COMBAT FUNCTIONS =====
    
    /** Main threat assessment function - analyzes all perceived threats */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AssessThreats();

    /** Update combat state based on current situation */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatState();

    /** Choose the best tactical approach for current situation */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ECombatTactic ChooseTactic();

    /** Execute the chosen tactic */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTactic(ECombatTactic Tactic);

    /** Force a specific combat state (for external triggers) */
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    // ===== THREAT MANAGEMENT =====
    
    /** Add a new threat to tracking */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void AddThreat(AActor* ThreatActor, EThreatLevel ThreatLevel = EThreatLevel::Moderate);

    /** Remove a threat from tracking */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    void RemoveThreat(AActor* ThreatActor);

    /** Get the highest priority threat */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    AActor* GetPrimaryThreat() const;

    /** Calculate threat score for an actor */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    float CalculateThreatScore(AActor* ThreatActor) const;

    /** Check if an actor is considered a threat */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    bool IsThreat(AActor* Actor) const;

    /** Get all current threats */
    UFUNCTION(BlueprintCallable, Category = "Threat Management")
    TArray<AActor*> GetAllThreats() const;

    // ===== TACTICAL POSITIONING =====
    
    /** Find the best tactical position relative to target */
    UFUNCTION(BlueprintCallable, Category = "Tactical Positioning")
    FVector FindTacticalPosition(AActor* Target, ECombatTactic Tactic);

    /** Find the best escape route from current position */
    UFUNCTION(BlueprintCallable, Category = "Tactical Positioning")
    FVector FindEscapeRoute();

    /** Find an ambush position for target */
    UFUNCTION(BlueprintCallable, Category = "Tactical Positioning")
    FVector FindAmbushPosition(AActor* Target);

    /** Check if current position is tactically sound */
    UFUNCTION(BlueprintCallable, Category = "Tactical Positioning")
    bool IsPositionTacticallySound() const;

    /** Get distance to maintain from target based on archetype */
    UFUNCTION(BlueprintCallable, Category = "Tactical Positioning")
    float GetOptimalCombatDistance(AActor* Target) const;

    // ===== PACK COORDINATION =====
    
    /** Join a pack with the specified leader */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void JoinPack(UCombatAIComponent* Leader);

    /** Leave current pack */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void LeavePack();

    /** Add a member to this pack (if leader) */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AddPackMember(UCombatAIComponent* Member);

    /** Remove a member from this pack */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void RemovePackMember(UCombatAIComponent* Member);

    /** Coordinate pack attack on target */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void CoordinatePackAttack(AActor* Target);

    /** Alert all pack members of threat */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    void AlertPack(AActor* Threat);

    /** Get pack formation position for this member */
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    FVector GetPackFormationPosition() const;

    // ===== COMBAT QUERIES =====
    
    /** Check if creature should engage in combat */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool ShouldEngageInCombat() const;

    /** Check if creature should flee */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool ShouldFlee() const;

    /** Check if creature can attack target */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool CanAttackTarget(AActor* Target) const;

    /** Check if target is within attack range */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool IsTargetInRange(AActor* Target) const;

    /** Check if creature has line of sight to target */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    bool HasLineOfSight(AActor* Target) const;

    /** Get current combat effectiveness (0.0 to 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Combat Queries")
    float GetCombatEffectiveness() const;

    // ===== EVENTS =====
    
    /** Called when combat state changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnCombatStateChanged(ECombatState OldState, ECombatState NewState);

    /** Called when a new threat is detected */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnThreatDetected(AActor* ThreatActor, EThreatLevel ThreatLevel);

    /** Called when a threat is lost */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnThreatLost(AActor* ThreatActor);

    /** Called when tactic changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnTacticChanged(ECombatTactic OldTactic, ECombatTactic NewTactic);

    /** Called when pack status changes */
    UFUNCTION(BlueprintImplementableEvent, Category = "Combat Events")
    void OnPackStatusChanged(bool bJoinedPack, UCombatAIComponent* PackLeader);

protected:
    // ===== INTERNAL FUNCTIONS =====
    
    void InitializeCombatAI();
    void UpdateThreatAssessments(float DeltaTime);
    void UpdatePackCoordination(float DeltaTime);
    void ProcessCombatLogic(float DeltaTime);
    void CleanupOldThreats(float DeltaTime);
    
    FThreatAssessment* FindThreatAssessment(AActor* Actor);
    void UpdateThreatAssessment(FThreatAssessment& Assessment, float DeltaTime);
    
    bool IsArchetypePredator() const;
    bool IsArchetypeHerbivore() const;
    bool IsArchetypePackAnimal() const;
    
    ECombatTactic GetArchetypeDefaultTactic() const;
    ECombatTactic GetFlightTactic() const;
    ECombatTactic GetAgggressiveTactic() const;
    ECombatTactic GetDefensiveTactic() const;

private:
    // ===== CACHED REFERENCES =====
    
    UPROPERTY()
    class AAIController* OwnerController;
    
    UPROPERTY()
    class APawn* OwnerPawn;
    
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
    
    UPROPERTY()
    class UAIPerceptionComponent* PerceptionComponent;

    // ===== TIMING AND OPTIMIZATION =====
    
    float LastThreatAssessmentTime = 0.0f;
    float ThreatAssessmentInterval = 0.2f;
    
    float LastTacticUpdateTime = 0.0f;
    float TacticUpdateInterval = 0.5f;
    
    float LastPackCoordinationTime = 0.0f;
    float PackCoordinationInterval = 1.0f;

    // ===== STATE TRACKING =====
    
    ECombatState PreviousCombatState = ECombatState::Passive;
    ECombatTactic PreviousTactic = ECombatTactic::StandGround;
    
    float StateChangeTime = 0.0f;
    float CurrentStateTime = 0.0f;
    
    bool bCombatAIInitialized = false;
    bool bInCombat = false;
};