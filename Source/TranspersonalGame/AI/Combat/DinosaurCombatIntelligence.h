#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "EnvironmentalQuery/EnvQueryManager.h"
#include "CombatAITypes.h"
#include "../DinosaurArchetypes.h"
#include "DinosaurCombatIntelligence.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class UAIPerceptionComponent;

/**
 * Combat intelligence levels - affects decision complexity and reaction time
 */
UENUM(BlueprintType)
enum class ECombatIntelligenceLevel : uint8
{
    Instinctual     UMETA(DisplayName = "Instinctual"),      // Basic predator/prey responses
    Tactical        UMETA(DisplayName = "Tactical"),         // Uses terrain and positioning
    Strategic       UMETA(DisplayName = "Strategic"),        // Plans multi-step attacks
    Adaptive        UMETA(DisplayName = "Adaptive"),         // Learns from encounters
    Apex            UMETA(DisplayName = "Apex")              // Master tactician
};

/**
 * Combat memory - tracks past encounters and learned behaviors
 */
USTRUCT(BlueprintType)
struct FCombatMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* EncounteredActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatAssessment = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SuccessfulTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> FailedTactics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EncounterCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastEncounterTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWasVictorious = false;

    FCombatMemory()
    {
        EncounteredActor = nullptr;
        ThreatAssessment = 0.5f;
        EncounterCount = 0;
        LastEncounterTime = 0.0f;
        bWasVictorious = false;
    }
};

/**
 * Tactical decision data structure
 */
USTRUCT(BlueprintType)
struct FTacticalDecision
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TacticName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SuccessScore = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetPosition = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ExecutionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredConditions;

    FTacticalDecision()
    {
        SuccessScore = 0.0f;
        TargetPosition = FVector::ZeroVector;
        ExecutionTime = 0.0f;
    }
};

/**
 * Advanced Combat Intelligence Component
 * Provides sophisticated AI decision-making for dinosaur combat
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDinosaurCombatIntelligence : public UActorComponent
{
    GENERATED_BODY()

public:
    UDinosaurCombatIntelligence();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Intelligence Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Intelligence")
    ECombatIntelligenceLevel IntelligenceLevel = ECombatIntelligenceLevel::Tactical;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Intelligence")
    float DecisionMakingSpeed = 1.0f; // Multiplier for how fast AI makes decisions

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Intelligence")
    float AdaptationRate = 0.1f; // How quickly AI learns from encounters

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Intelligence")
    int32 MaxMemoryEntries = 20;

    // Tactical Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    float TacticalThinkingDelay = 0.5f; // Time spent analyzing before acting

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    bool bUsesFeints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    bool bUsesAmbushes = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    bool bUsesTerrainTactics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Behavior")
    bool bCanLearnFromFailure = true;

    // Combat Memory
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Memory")
    TArray<FCombatMemory> CombatMemories;

    // Current Tactical State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    FTacticalDecision CurrentTactic;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    float TacticalConfidence = 1.0f; // 0.0 = Uncertain, 1.0 = Very confident

public:
    // Intelligence Analysis
    UFUNCTION(BlueprintCallable, Category = "Combat Intelligence")
    FTacticalDecision AnalyzeTacticalSituation(AActor* Target, const TArray<AActor*>& NearbyThreats);

    UFUNCTION(BlueprintCallable, Category = "Combat Intelligence")
    float EvaluateTacticalAdvantage(const FVector& Position, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Intelligence")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat Intelligence")
    bool ShouldRetreatFromCombat(AActor* Target, float CurrentHealthPercentage);

    // Memory Management
    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    void RecordCombatEncounter(AActor* Opponent, const FString& TacticUsed, bool bWasSuccessful);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    FCombatMemory GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Combat Memory")
    float GetThreatAssessmentFromMemory(AActor* Actor);

    // Tactical Decision Making
    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    FString SelectOptimalTactic(AActor* Target, const TArray<FString>& AvailableTactics);

    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    FVector FindOptimalAttackPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    bool ShouldUseAmbushTactic(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical Decisions")
    bool ShouldUseFeintTactic(AActor* Target);

    // Adaptive Learning
    UFUNCTION(BlueprintCallable, Category = "Adaptive Learning")
    void AdaptToFailedTactic(const FString& FailedTactic, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Learning")
    void ReinforceTacticSuccess(const FString& SuccessfulTactic, AActor* Target);

    // Environmental Analysis
    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    bool HasTerrainAdvantage(const FVector& Position, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    TArray<FVector> FindCoverPositions(float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Analysis")
    FVector FindFlankingPosition(AActor* Target);

    // Pack Coordination Intelligence
    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    FTacticalDecision CoordinatePackAttack(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    bool ShouldCallForBackup(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Coordination")
    FVector GetOptimalPackPosition(const TArray<AActor*>& PackMembers, AActor* Target);

    // Intelligence Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTacticalDecisionMade, const FTacticalDecision&, Decision, AActor*, Target);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTacticalDecisionMade OnTacticalDecisionMade;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatLearning, AActor*, Opponent, FString, Tactic, bool, bWasSuccessful);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCombatLearning OnCombatLearning;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Combat Intelligence")
    ECombatIntelligenceLevel GetIntelligenceLevel() const { return IntelligenceLevel; }

    UFUNCTION(BlueprintPure, Category = "Combat Intelligence")
    float GetTacticalConfidence() const { return TacticalConfidence; }

    UFUNCTION(BlueprintPure, Category = "Combat Intelligence")
    bool IsCurrentlyThinking() const { return bIsThinking; }

private:
    // Internal state
    bool bIsThinking = false;
    float ThinkingStartTime = 0.0f;
    
    // Intelligence calculation helpers
    float CalculateIntelligenceModifier() const;
    float CalculateTacticalComplexity(AActor* Target, const TArray<AActor*>& NearbyThreats) const;
    void UpdateTacticalConfidence(bool bLastTacticSuccessful);
    
    // Memory helpers
    void CleanupOldMemories();
    FCombatMemory* FindOrCreateMemory(AActor* Actor);
    
    // Tactical analysis helpers
    float AnalyzePositionalAdvantage(const FVector& Position, AActor* Target) const;
    float AnalyzeTerrainAdvantage(const FVector& Position) const;
    float AnalyzeEscapeRoutes(const FVector& Position) const;
    
    // Decision making helpers
    TArray<FString> GetAvailableTactics() const;
    float EvaluateTacticEffectiveness(const FString& Tactic, AActor* Target) const;
    bool MeetsTacticRequirements(const FString& Tactic, AActor* Target) const;
};