#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTags.h"
#include "CombatAIController.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UBehaviorTree;
class ACombatAICharacter;
class UEnvQuery;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive,        // Peaceful behavior - grazing, resting
    Alert,          // Noticed something suspicious
    Hunting,        // Actively hunting prey
    Territorial,    // Defending territory
    Fleeing,        // Running from threat
    Aggressive,     // Direct combat engagement
    Stalking        // Following target stealthily
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,
    Low,
    Medium,
    High,
    Extreme
};

USTRUCT(BlueprintType)
struct FCombatPersonality
{
    GENERATED_BODY()

    // Aggression level (0.0 = passive, 1.0 = extremely aggressive)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    // Curiosity level (how likely to investigate disturbances)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    // Fear threshold (how easily spooked)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearThreshold = 0.5f;

    // Territorial behavior strength
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    // Pack behavior tendency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PackMentality = 0.5f;

    // Persistence in pursuit
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Persistence = 0.5f;

    FCombatPersonality()
    {
        // Generate random personality traits
        Aggression = FMath::RandRange(0.1f, 0.9f);
        Curiosity = FMath::RandRange(0.1f, 0.9f);
        FearThreshold = FMath::RandRange(0.2f, 0.8f);
        Territoriality = FMath::RandRange(0.1f, 0.9f);
        PackMentality = FMath::RandRange(0.1f, 0.9f);
        Persistence = FMath::RandRange(0.3f, 0.9f);
    }
};

USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    // Last known player location
    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    // Time since last player sighting
    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastPlayerSighting = 0.0f;

    // Number of times player was spotted
    UPROPERTY(BlueprintReadWrite)
    int32 PlayerSpottingCount = 0;

    // Threat level assessment of player
    UPROPERTY(BlueprintReadWrite)
    EThreatLevel PlayerThreatLevel = EThreatLevel::None;

    // Locations where player was last seen
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> PlayerSightingHistory;

    // Time stamps of sightings
    UPROPERTY(BlueprintReadWrite)
    TArray<float> SightingTimeStamps;
};

/**
 * AI Controller for combat-focused dinosaur AI
 * Implements tactical combat behavior with personality-driven decisions
 */
UCLASS()
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Behavior Tree
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    // Combat personality traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatPersonality Personality;

    // Combat memory system
    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    FCombatMemory CombatMemory;

    // Current combat state
    UPROPERTY(BlueprintReadWrite, Category = "Combat AI")
    ECombatState CurrentCombatState = ECombatState::Passive;

    // EQS Queries for tactical positioning
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS")
    UEnvQuery* FindCoverQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS")
    UEnvQuery* FindAmbushQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS")
    UEnvQuery* FindFlankingQuery;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "EQS")
    UEnvQuery* FindEscapeQuery;

    // Gameplay Tags for combat states
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Tags")
    FGameplayTag CombatStateTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat Tags")
    FGameplayTag ThreatLevelTag;

public:
    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Combat state management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Threat assessment
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    EThreatLevel AssessThreatLevel(AActor* Target);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdatePlayerMemory(const FVector& PlayerLocation);

    UFUNCTION(BlueprintPure, Category = "Combat AI")
    FVector GetLastKnownPlayerLocation() const { return CombatMemory.LastKnownPlayerLocation; }

    // Tactical decision making
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFleeFromTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldStalkTarget(AActor* Target);

    // EQS query execution
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalQuery(UEnvQuery* Query, FName BlackboardKey);

    // Personality-driven behavior modifiers
    UFUNCTION(BlueprintPure, Category = "Combat AI")
    float GetPersonalityModifier(const FString& TraitName) const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ModifyBehaviorByPersonality();

private:
    // Internal state tracking
    float StateChangeTimer = 0.0f;
    float LastPerceptionUpdate = 0.0f;
    
    // Combat effectiveness tracking
    float CombatEffectiveness = 1.0f;
    int32 SuccessfulAttacks = 0;
    int32 FailedAttacks = 0;

    // Helper functions
    void InitializePersonality();
    void UpdateCombatMemory(float DeltaTime);
    void EvaluateCombatSituation();
    void HandleStateTransition(ECombatState NewState);
    float CalculateDistanceToPlayer() const;
    bool IsPlayerInLineOfSight() const;
    void UpdateBlackboardValues();
};