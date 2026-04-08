// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "../NPCBehavior/NPCBehaviorSystem.h"
#include "../Characters/CharacterArtistSystem.h"
#include "CombatAISystem.generated.h"

/**
 * Combat Archetypes - defines fundamental combat behavior patterns
 * Each dinosaur species has a primary archetype that drives their combat AI
 */
UENUM(BlueprintType)
enum class ECombatArchetype : uint8
{
    // Predator Archetypes
    ApexPredator        UMETA(DisplayName = "Apex Predator"),      // T-Rex, Giganotosaurus
    PackHunter          UMETA(DisplayName = "Pack Hunter"),        // Velociraptors, Deinonychus
    AmbushPredator      UMETA(DisplayName = "Ambush Predator"),    // Carnotaurus, Baryonyx
    OpportunistHunter   UMETA(DisplayName = "Opportunist Hunter"), // Compsognathus, Coelophysis
    
    // Herbivore Archetypes
    TankDefender        UMETA(DisplayName = "Tank Defender"),      // Triceratops, Ankylosaurus
    HerdProtector       UMETA(DisplayName = "Herd Protector"),     // Parasaurolophus, Edmontosaurus
    FlightResponse      UMETA(DisplayName = "Flight Response"),    // Gallimimus, Ornithomimus
    TerritorialGuard    UMETA(DisplayName = "Territorial Guard"),  // Stegosaurus, Kentrosaurus
    
    // Special Archetypes
    ScavengerOpportunist UMETA(DisplayName = "Scavenger"),        // Scavenging behavior
    AquaticHunter       UMETA(DisplayName = "Aquatic Hunter"),     // Spinosaurus, Suchomimus
    AerialPredator      UMETA(DisplayName = "Aerial Predator"),    // Pteranodon, Quetzalcoatlus
    
    MAX                 UMETA(Hidden)
};

/**
 * Combat States that define current combat behavior
 */
UENUM(BlueprintType)
enum class ECombatState : uint8
{
    // Non-Combat States
    Passive             UMETA(DisplayName = "Passive"),
    Unaware             UMETA(DisplayName = "Unaware"),
    Investigating       UMETA(DisplayName = "Investigating"),
    
    // Pre-Combat States
    Alerted             UMETA(DisplayName = "Alerted"),
    Stalking            UMETA(DisplayName = "Stalking"),
    Positioning         UMETA(DisplayName = "Positioning"),
    Intimidating        UMETA(DisplayName = "Intimidating"),
    
    // Active Combat States
    Engaging            UMETA(DisplayName = "Engaging"),
    Attacking           UMETA(DisplayName = "Attacking"),
    Defending           UMETA(DisplayName = "Defending"),
    Flanking            UMETA(DisplayName = "Flanking"),
    
    // Reactive States
    Wounded             UMETA(DisplayName = "Wounded"),
    Retreating          UMETA(DisplayName = "Retreating"),
    Regrouping          UMETA(DisplayName = "Regrouping"),
    Enraged             UMETA(DisplayName = "Enraged"),
    
    // Special States
    Hunting             UMETA(DisplayName = "Hunting"),
    Feeding             UMETA(DisplayName = "Feeding"),
    Protecting          UMETA(DisplayName = "Protecting"),
    Fleeing             UMETA(DisplayName = "Fleeing"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Combat Tactics - specific behaviors within combat states
 */
UENUM(BlueprintType)
enum class ECombatTactic : uint8
{
    // Attack Tactics
    DirectAssault       UMETA(DisplayName = "Direct Assault"),
    CircleAndStrike     UMETA(DisplayName = "Circle and Strike"),
    ChargeAttack        UMETA(DisplayName = "Charge Attack"),
    AmbushStrike        UMETA(DisplayName = "Ambush Strike"),
    PounceAttack        UMETA(DisplayName = "Pounce Attack"),
    
    // Defensive Tactics
    StandGround         UMETA(DisplayName = "Stand Ground"),
    CreateDistance      UMETA(DisplayName = "Create Distance"),
    UseEnvironment      UMETA(DisplayName = "Use Environment"),
    CallForHelp         UMETA(DisplayName = "Call for Help"),
    
    // Group Tactics
    CoordinatedAttack   UMETA(DisplayName = "Coordinated Attack"),
    Pincer              UMETA(DisplayName = "Pincer Movement"),
    Distraction         UMETA(DisplayName = "Distraction"),
    HerdFormation       UMETA(DisplayName = "Herd Formation"),
    
    // Evasion Tactics
    ZigzagEscape        UMETA(DisplayName = "Zigzag Escape"),
    BreakLineOfSight    UMETA(DisplayName = "Break Line of Sight"),
    UseObstacles        UMETA(DisplayName = "Use Obstacles"),
    ScatterPattern      UMETA(DisplayName = "Scatter Pattern"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Threat Assessment Levels
 */
UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None                UMETA(DisplayName = "No Threat"),
    Minimal             UMETA(DisplayName = "Minimal Threat"),
    Low                 UMETA(DisplayName = "Low Threat"),
    Moderate            UMETA(DisplayName = "Moderate Threat"),
    High                UMETA(DisplayName = "High Threat"),
    Extreme             UMETA(DisplayName = "Extreme Threat"),
    Lethal              UMETA(DisplayName = "Lethal Threat"),
    
    MAX                 UMETA(Hidden)
};

/**
 * Combat Capabilities - what this creature can do in combat
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatCapabilities
{
    GENERATED_BODY()

    // Physical Capabilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physical\")
    float MaxSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physical\")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physical\")
    float JumpHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Physical\")
    float SwimSpeed;

    // Attack Capabilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Attack\")
    float AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Attack\")
    float AttackDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Attack\")
    float AttackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Attack\")
    float KnockbackForce;

    // Sensory Capabilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Senses\")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Senses\")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Senses\")
    float SmellRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Senses\")
    float SightAngle;

    // Tactical Capabilities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanClimb;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanSwim;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanFly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanDigBurrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanUseTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Tactical\")
    bool CanCoordinate;

    FCombatCapabilities()
    {
        MaxSpeed = 600.0f;
        TurnRate = 90.0f;
        JumpHeight = 100.0f;
        SwimSpeed = 300.0f;
        AttackRange = 200.0f;
        AttackDamage = 50.0f;
        AttackSpeed = 1.0f;
        KnockbackForce = 500.0f;
        SightRange = 2000.0f;
        HearingRange = 1500.0f;
        SmellRange = 1000.0f;
        SightAngle = 90.0f;
        CanClimb = false;
        CanSwim = false;
        CanFly = false;
        CanDigBurrow = false;
        CanUseTools = false;
        CanCoordinate = false;
    }
};

/**
 * Threat Assessment Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FThreatAssessment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    AActor* ThreatActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    EThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    float ThreatScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    float DistanceToThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    bool bIsMoving;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat\")
    FVector MovementDirection;

    FThreatAssessment()
    {
        ThreatActor = nullptr;
        ThreatLevel = EThreatLevel::None;
        ThreatScore = 0.0f;
        LastKnownLocation = FVector::ZeroVector;
        LastSeenTime = 0.0f;
        DistanceToThreat = 0.0f;
        bIsVisible = false;
        bIsMoving = false;
        MovementDirection = FVector::ZeroVector;
    }
};

/**
 * Combat Decision Data - used by AI to make tactical decisions
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatDecision
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    ECombatTactic ChosenTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    float ConfidenceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    float DecisionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision\")
    TArray<FString> ReasoningFactors;

    FCombatDecision()
    {
        ChosenTactic = ECombatTactic::DirectAssault;
        ConfidenceLevel = 0.5f;
        TargetLocation = FVector::ZeroVector;
        TargetActor = nullptr;
        DecisionTime = 0.0f;
    }
};

/**
 * Combat Memory - remembers past combat encounters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    AActor* OpponentActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    ECombatTactic LastSuccessfulTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    ECombatTactic LastFailedTactic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    float EncounterOutcome; // -1.0 = defeat, 0.0 = draw, 1.0 = victory

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    float MemoryStrength; // How well this encounter is remembered

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Memory\")
    FVector EncounterLocation;

    FCombatMemory()
    {
        OpponentActor = nullptr;
        LastSuccessfulTactic = ECombatTactic::DirectAssault;
        LastFailedTactic = ECombatTactic::DirectAssault;
        EncounterOutcome = 0.0f;
        MemoryStrength = 1.0f;
        Timestamp = 0.0f;
        EncounterLocation = FVector::ZeroVector;
    }
};

/**
 * Main Combat AI Component
 * Handles all combat-related AI behavior for dinosaurs and NPCs
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE COMBAT PROPERTIES ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat AI\")
    ECombatArchetype CombatArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat AI\")
    ECombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat AI\")
    FCombatCapabilities Capabilities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat AI\")
    UBehaviorTree* CombatBehaviorTree;

    // === THREAT ASSESSMENT ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Threat Assessment\")
    TArray<FThreatAssessment> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Threat Assessment\")
    FThreatAssessment PrimaryThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Threat Assessment\")
    float ThreatUpdateInterval;

    // === DECISION MAKING ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Decision Making\")
    FCombatDecision CurrentDecision;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision Making\")
    float DecisionUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Decision Making\")
    float ConfidenceThreshold;

    // === COMBAT MEMORY ===

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = \"Combat Memory\")
    TArray<FCombatMemory> CombatMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat Memory\")
    int32 MaxMemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Combat Memory\")
    float MemoryDecayRate;

    // === PERSONALITY INFLUENCE ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Personality\")
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Personality\")
    float Cautiousness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Personality\")
    float Territoriality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Personality\")
    float PackLoyalty;

    // === ENVIRONMENTAL AWARENESS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")
    float EnvironmentScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")
    float CoverPreference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = \"Environment\")
    float HeightAdvantagePreference;

    // === CORE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = \"Combat AI\")
    void UpdateThreatAssessment();

    UFUNCTION(BlueprintCallable, Category = \"Combat AI\")
    void MakeCombatDecision();

    UFUNCTION(BlueprintCallable, Category = \"Combat AI\")
    void ExecuteCurrentDecision();

    UFUNCTION(BlueprintCallable, Category = \"Combat AI\")
    void TransitionToState(ECombatState NewState);

    // === THREAT ASSESSMENT FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = \"Threat Assessment\")
    float CalculateThreatScore(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = \"Threat Assessment\")
    EThreatLevel DetermineThreatLevel(float ThreatScore);

    UFUNCTION(BlueprintCallable, Category = \"Threat Assessment\")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = \"Threat Assessment\")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = \"Threat Assessment\")
    FThreatAssessment GetPrimaryThreat();

    // === DECISION MAKING FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = \"Decision Making\")
    ECombatTactic SelectBestTactic();

    UFUNCTION(BlueprintCallable, Category = \"Decision Making\")
    float EvaluateTactic(ECombatTactic Tactic);

    UFUNCTION(BlueprintCallable, Category = \"Decision Making\")
    bool CanExecuteTactic(ECombatTactic Tactic);

    UFUNCTION(BlueprintCallable, Category = \"Decision Making\")
    FVector FindOptimalPosition(ECombatTactic Tactic);

    // === MEMORY FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = \"Combat Memory\")
    void RecordCombatOutcome(AActor* Opponent, ECombatTactic UsedTactic, float Outcome);

    UFUNCTION(BlueprintCallable, Category = \"Combat Memory\")
    FCombatMemory GetMemoryOfOpponent(AActor* Opponent);

    UFUNCTION(BlueprintCallable, Category = \"Combat Memory\")
    void UpdateMemoryDecay(float DeltaTime);

    // === ENVIRONMENTAL ANALYSIS ===

    UFUNCTION(BlueprintCallable, Category = \"Environment\")
    TArray<FVector> FindCoverPositions();

    UFUNCTION(BlueprintCallable, Category = \"Environment\")
    FVector FindFlankingPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = \"Environment\")
    bool HasLineOfSight(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = \"Environment\")
    float CalculateTerrainAdvantage(FVector Position);

    // === ARCHETYPE-SPECIFIC BEHAVIORS ===

    UFUNCTION(BlueprintCallable, Category = \"Archetype Behavior\")
    void ExecuteApexPredatorBehavior();

    UFUNCTION(BlueprintCallable, Category = \"Archetype Behavior\")
    void ExecutePackHunterBehavior();

    UFUNCTION(BlueprintCallable, Category = \"Archetype Behavior\")
    void ExecuteAmbushPredatorBehavior();

    UFUNCTION(BlueprintCallable, Category = \"Archetype Behavior\")
    void ExecuteHerbivoreDefenseBehavior();

    // === EVENTS ===

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatStateChanged, ECombatState, OldState, ECombatState, NewState);
    UPROPERTY(BlueprintAssignable, Category = \"Events\")
    FOnCombatStateChanged OnCombatStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnThreatDetected, AActor*, ThreatActor);
    UPROPERTY(BlueprintAssignable, Category = \"Events\")
    FOnThreatDetected OnThreatDetected;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDecisionMade, FCombatDecision, Decision);
    UPROPERTY(BlueprintAssignable, Category = \"Events\")
    FOnDecisionMade OnDecisionMade;

private:
    // Internal state tracking
    float LastThreatUpdateTime;
    float LastDecisionUpdateTime;
    bool bInCombat;
    
    // Helper functions
    void InitializeArchetypeDefaults();
    void UpdateCombatState(float DeltaTime);
    void ProcessPerceptionData();
    FVector PredictTargetMovement(AActor* Target, float PredictionTime);
    bool IsValidCombatTarget(AActor* Target);
};