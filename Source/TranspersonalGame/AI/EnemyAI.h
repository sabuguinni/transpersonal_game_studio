// EnemyAI.h
// Sistema de IA inimiga que responde aos estados de consciência
#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "../Consciousness/ConsciousnessTypes.h"
#include "../Combat/CombatSystem.h"
#include "EnemyAI.generated.h"

UENUM(BlueprintType)
enum class EEnemyType : uint8
{
    Shadow          UMETA(DisplayName = "Shadow Entity"),
    Guardian        UMETA(DisplayName = "Spiritual Guardian"),
    Teacher         UMETA(DisplayName = "Wisdom Teacher"),
    Challenger      UMETA(DisplayName = "Consciousness Challenger"),
    Transformer     UMETA(DisplayName = "Transformative Being")
};

UENUM(BlueprintType)
enum class EEnemyBehaviorState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Observing       UMETA(DisplayName = "Observing"),
    Testing         UMETA(DisplayName = "Testing Player"),
    Teaching        UMETA(DisplayName = "Teaching"),
    Challenging     UMETA(DisplayName = "Challenging"),
    Hostile         UMETA(DisplayName = "Hostile"),
    Transcending    UMETA(DisplayName = "Transcending")
};

USTRUCT(BlueprintType)
struct FEnemyConsciousnessProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState MinimumState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState MaximumState = EConsciousnessState::Cosmic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualResonanceRange = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRespondsToPlayerConsciousness = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanEvolveConsciousness = false;
};

USTRUCT(BlueprintType)
struct FEnemyBehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TeachingProbability = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransformativeProbability = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessInfluence = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EAttackType> PreferredAttackTypes;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnemyBehaviorChanged, EEnemyBehaviorState, NewState, FString, Reason, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyConsciousnessEvolved, EConsciousnessState, NewState, float, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEnemyTeaching, FString, TeachingMessage, EConsciousnessState, RequiredState, float, WisdomValue);

UCLASS()
class TRANSPERSONALGAME_API AEnemyAI : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Behavior Management
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void SetBehaviorState(EEnemyBehaviorState NewState, const FString& Reason = TEXT(""), float Intensity = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    EEnemyBehaviorState GetBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void UpdateBehaviorFromPlayerConsciousness(EConsciousnessState PlayerState, float PlayerLevel);

    // Consciousness Evolution
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void EvolveConsciousness(float EvolutionAmount);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool CanEvolveToState(EConsciousnessState TargetState) const;

    // Teaching and Transformation
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void AttemptTeaching(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void OfferTransformation(APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool ShouldEngageInCombat(APawn* Player) const;

    // Combat Integration
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ExecuteConsciousAttack(APawn* Target);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    EAttackType SelectOptimalAttackType(EConsciousnessState PlayerState) const;

    // Spiritual Interaction
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    float CalculateResonanceWithPlayer(APawn* Player) const;

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool IsPlayerReadyForChallenge(APawn* Player) const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void SetEnemyType(EEnemyType NewType);

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    EEnemyType GetEnemyType() const { return EnemyType; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Enemy AI Events")
    FOnEnemyBehaviorChanged OnBehaviorChanged;

    UPROPERTY(BlueprintAssignable, Category = "Enemy AI Events")
    FOnEnemyConsciousnessEvolved OnConsciousnessEvolved;

    UPROPERTY(BlueprintAssignable, Category = "Enemy AI Events")
    FOnEnemyTeaching OnTeaching;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Enemy Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Config")
    EEnemyType EnemyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Config")
    FEnemyConsciousnessProfile ConsciousnessProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Config")
    FEnemyBehaviorConfig BehaviorConfig;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State")
    EEnemyBehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State")
    EConsciousnessState CurrentConsciousnessState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State")
    float BehaviorIntensity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy State")
    float LastPlayerResonance;

    // Behavior Trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    class UBehaviorTree* DefaultBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Behavior")
    TMap<EEnemyBehaviorState, class UBehaviorTree*> BehaviorTrees;

    // Teaching System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teaching")
    TArray<FString> TeachingMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teaching")
    TMap<EConsciousnessState, TArray<FString>> ConsciousnessTeachings;

private:
    void InitializeAIComponents();
    void InitializeConsciousnessProfile();
    void InitializeBehaviorConfig();
    void UpdateBlackboardValues();
    void ProcessConsciousnessEvolution(float DeltaTime);
    void EvaluatePlayerInteraction(APawn* Player);
    
    FString GenerateTeachingMessage(EConsciousnessState RequiredState) const;
    bool ValidateConsciousnessTransition(EConsciousnessState NewState) const;
    
    float ConsciousnessEvolutionTimer;
    float PlayerEvaluationTimer;
    bool bHasMetPlayer;
};