#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "CombatAITypes.h"
#include "AdvancedCombatAI.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class UTacticalCombatAnalyzer;
class UCombatTensionManager;
class UPlayerBehaviorAnalyzer;

/**
 * Advanced Combat AI System
 * 
 * Core Philosophy: "Every encounter should feel like a conversation between predator and prey,
 * where the player never knows if they'll win until the last second."
 * 
 * Key Features:
 * - Dynamic threat assessment with behavioral adaptation
 * - Tension-based engagement management
 * - Player behavior learning and counter-adaptation
 * - Coordinated pack tactics with emergent behaviors
 * - Environmental exploitation and tactical positioning
 * - Unpredictable but fair combat encounters
 */

UENUM(BlueprintType)
enum class ECombatTensionLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Unease      UMETA(DisplayName = "Growing Unease"),
    Tension     UMETA(DisplayName = "High Tension"),
    Terror      UMETA(DisplayName = "Terror"),
    Panic       UMETA(DisplayName = "Panic"),
    Relief      UMETA(DisplayName = "Relief")
};

UENUM(BlueprintType)
enum class ECombatPersonalityType : uint8
{
    Stalker         UMETA(DisplayName = "Patient Stalker"),
    Ambusher        UMETA(DisplayName = "Ambush Predator"),
    Berserker       UMETA(DisplayName = "Aggressive Berserker"),
    Tactician       UMETA(DisplayName = "Tactical Coordinator"),
    Opportunist     UMETA(DisplayName = "Opportunistic Hunter"),
    Territorial     UMETA(DisplayName = "Territorial Defender"),
    PackHunter      UMETA(DisplayName = "Pack Hunter"),
    Scavenger       UMETA(DisplayName = "Opportunistic Scavenger")
};

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    Unaware         UMETA(DisplayName = "Unaware"),
    Detection       UMETA(DisplayName = "Detection Phase"),
    Assessment      UMETA(DisplayName = "Threat Assessment"),
    Positioning     UMETA(DisplayName = "Tactical Positioning"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Engagement      UMETA(DisplayName = "Active Engagement"),
    Pursuit         UMETA(DisplayName = "Pursuit"),
    Retreat         UMETA(DisplayName = "Strategic Retreat"),
    Victory         UMETA(DisplayName = "Victory"),
    Defeat          UMETA(DisplayName = "Defeat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAdvancedCombatPersonality
{
    GENERATED_BODY()

    // Core personality traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Patience = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Unpredictability = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialCoordination = 0.5f;

    // Behavioral preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECombatPersonalityType PersonalityType = ECombatPersonalityType::Opportunist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PreferredEngagementDistance = 0.5f; // 0 = close combat, 1 = long range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbushPreference = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FlankingPreference = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RetreatThreshold = 0.3f;

    // Adaptation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LearningRate = 0.1f; // How quickly this AI adapts to player behavior

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MemoryRetention = 0.8f; // How well this AI remembers past encounters

    FAdvancedCombatPersonality()
    {
        GenerateRandomPersonality();
    }

    void GenerateRandomPersonality()
    {
        // Generate varied but coherent personalities
        PersonalityType = static_cast<ECombatPersonalityType>(FMath::RandRange(0, 7));
        
        switch (PersonalityType)
        {
        case ECombatPersonalityType::Stalker:
            Aggression = FMath::RandRange(0.3f, 0.6f);
            Patience = FMath::RandRange(0.7f, 0.95f);
            Intelligence = FMath::RandRange(0.6f, 0.9f);
            Unpredictability = FMath::RandRange(0.2f, 0.4f);
            AmbushPreference = FMath::RandRange(0.6f, 0.9f);
            break;
            
        case ECombatPersonalityType::Berserker:
            Aggression = FMath::RandRange(0.8f, 1.0f);
            Patience = FMath::RandRange(0.1f, 0.3f);
            Intelligence = FMath::RandRange(0.2f, 0.5f);
            Unpredictability = FMath::RandRange(0.4f, 0.7f);
            PreferredEngagementDistance = FMath::RandRange(0.0f, 0.3f);
            break;
            
        case ECombatPersonalityType::Tactician:
            Aggression = FMath::RandRange(0.4f, 0.7f);
            Patience = FMath::RandRange(0.6f, 0.9f);
            Intelligence = FMath::RandRange(0.8f, 1.0f);
            SocialCoordination = FMath::RandRange(0.7f, 1.0f);
            FlankingPreference = FMath::RandRange(0.6f, 0.9f);
            break;
            
        default:
            // Randomize for other types
            Aggression = FMath::RandRange(0.2f, 0.8f);
            Patience = FMath::RandRange(0.3f, 0.7f);
            Intelligence = FMath::RandRange(0.4f, 0.8f);
            break;
        }
        
        Unpredictability = FMath::RandRange(0.2f, 0.5f);
        LearningRate = FMath::RandRange(0.05f, 0.15f);
        MemoryRetention = FMath::RandRange(0.7f, 0.95f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    // Player behavior observations
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, float> PlayerBehaviorPatterns;

    // Successful tactics against this player
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> SuccessfulTactics;

    // Failed tactics against this player
    UPROPERTY(BlueprintReadOnly)
    TArray<FString> FailedTactics;

    // Environmental preferences observed
    UPROPERTY(BlueprintReadOnly)
    TArray<FVector> PlayerPreferredLocations;

    // Timing patterns
    UPROPERTY(BlueprintReadOnly)
    TMap<float, float> PlayerActivityPatterns; // Time of day -> Activity level

    // Last encounter data
    UPROPERTY(BlueprintReadOnly)
    float LastEncounterTime = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    bool LastEncounterWon = false;

    UPROPERTY(BlueprintReadOnly)
    FString LastUsedTactic;

    FCombatMemory()
    {
        Reset();
    }

    void Reset()
    {
        PlayerBehaviorPatterns.Empty();
        SuccessfulTactics.Empty();
        FailedTactics.Empty();
        PlayerPreferredLocations.Empty();
        PlayerActivityPatterns.Empty();
        LastEncounterTime = 0.0f;
        LastEncounterWon = false;
        LastUsedTactic = "";
    }
};

/**
 * Advanced Combat AI Controller
 * Implements sophisticated combat intelligence with focus on creating memorable encounters
 */
UCLASS()
class TRANSPERSONALGAME_API AAdvancedCombatAI : public AAIController
{
    GENERATED_BODY()

public:
    AAdvancedCombatAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Core AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UTacticalCombatAnalyzer* TacticalAnalyzer;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UCombatTensionManager* TensionManager;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UPlayerBehaviorAnalyzer* BehaviorAnalyzer;

    // AI Assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Assets")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI Assets")
    UBlackboardAsset* CombatBlackboard;

    // Combat Personality
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Personality")
    FAdvancedCombatPersonality CombatPersonality;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatPhase CurrentCombatPhase = ECombatPhase::Unaware;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    ECombatTensionLevel CurrentTensionLevel = ECombatTensionLevel::Calm;

    // Combat Memory and Learning
    UPROPERTY(BlueprintReadOnly, Category = "Combat Memory")
    FCombatMemory CombatMemory;

    // Current target and threat data
    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    TArray<AActor*> SecondaryTargets;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Target")
    TArray<AActor*> AlliedUnits;

    // Timing and state management
    UPROPERTY(BlueprintReadOnly, Category = "Combat Timing")
    float PhaseStartTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Timing")
    float LastDecisionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Combat Timing")
    float NextDecisionTime = 0.0f;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float DecisionUpdateInterval = 0.5f; // How often to make new tactical decisions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float TensionUpdateInterval = 0.1f; // How often to update tension levels

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float MaxEngagementDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    float MinEngagementDistance = 100.0f;

public:
    // Core Combat Interface
    UFUNCTION(BlueprintCallable, Category = "Combat Control")
    void SetCombatPhase(ECombatPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Combat Control")
    void SetPrimaryTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat Control")
    void AddSecondaryTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Control")
    void RemoveTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Control")
    void RegisterAlliedUnit(AActor* Ally);

    // Decision Making
    UFUNCTION(BlueprintCallable, Category = "Combat Decisions")
    void MakeTacticalDecision();

    UFUNCTION(BlueprintCallable, Category = "Combat Decisions")
    FString SelectOptimalTactic();

    UFUNCTION(BlueprintCallable, Category = "Combat Decisions")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Decisions")
    bool ShouldRetreat();

    // Learning and Adaptation
    UFUNCTION(BlueprintCallable, Category = "Combat Learning")
    void RecordPlayerBehavior(const FString& BehaviorType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Combat Learning")
    void RecordTacticResult(const FString& TacticName, bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Combat Learning")
    void AdaptToPlayerBehavior();

    // Tension Management
    UFUNCTION(BlueprintCallable, Category = "Tension Management")
    void UpdateTensionLevel();

    UFUNCTION(BlueprintCallable, Category = "Tension Management")
    void ModifyTension(float DeltaTension);

    UFUNCTION(BlueprintCallable, Category = "Tension Management")
    ECombatTensionLevel CalculateOptimalTensionLevel();

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombatPhase GetCombatPhase() const { return CurrentCombatPhase; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    ECombatTensionLevel GetTensionLevel() const { return CurrentTensionLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    FAdvancedCombatPersonality GetCombatPersonality() const { return CombatPersonality; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    FCombatMemory GetCombatMemory() const { return CombatMemory; }

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCombatPhaseChanged, ECombatPhase, OldPhase, ECombatPhase, NewPhase);
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatPhaseChanged OnCombatPhaseChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTensionLevelChanged, ECombatTensionLevel, OldLevel, ECombatTensionLevel, NewLevel);
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnTensionLevelChanged OnTensionLevelChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTacticSelected, FString, TacticName, AActor*, Target);
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnTacticSelected OnTacticSelected;

protected:
    // Internal logic
    void InitializeCombatSystems();
    void UpdateCombatLogic(float DeltaTime);
    void ProcessPhaseTransitions();
    void UpdateBlackboardValues();
    
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Phase-specific logic
    void HandleUnaware(float DeltaTime);
    void HandleDetection(float DeltaTime);
    void HandleAssessment(float DeltaTime);
    void HandlePositioning(float DeltaTime);
    void HandleStalking(float DeltaTime);
    void HandleEngagement(float DeltaTime);
    void HandlePursuit(float DeltaTime);
    void HandleRetreat(float DeltaTime);

    // Utility functions
    bool IsValidTarget(AActor* Target);
    float CalculateTargetPriority(AActor* Target);
    FVector GetOptimalPosition(AActor* Target, const FString& TacticName);
    bool HasLineOfSight(AActor* Target);
    float GetDistanceToTarget(AActor* Target);

private:
    // Internal state
    float LastTensionUpdate = 0.0f;
    FString CurrentTactic = "";
    FVector LastKnownTargetPosition = FVector::ZeroVector;
    float TargetLostTime = 0.0f;
    
    // Adaptation tracking
    TMap<FString, int32> TacticUsageCount;
    TMap<FString, float> TacticSuccessRate;
    float LastAdaptationTime = 0.0f;
};