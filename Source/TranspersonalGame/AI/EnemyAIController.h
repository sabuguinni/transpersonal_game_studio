#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Combat/TranspersonalCombatSystem.h"
#include "EnemyAIController.generated.h"

UENUM(BlueprintType)
enum class EEnemyAIState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Patrol          UMETA(DisplayName = "Patrol"),
    Investigate     UMETA(DisplayName = "Investigate"),
    Chase           UMETA(DisplayName = "Chase"),
    Combat          UMETA(DisplayName = "Combat"),
    Retreat         UMETA(DisplayName = "Retreat"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Shadow          UMETA(DisplayName = "Shadow")
};

UENUM(BlueprintType)
enum class EConsciousnessResponse : uint8
{
    Hostile         UMETA(DisplayName = "Hostile"),
    Neutral         UMETA(DisplayName = "Neutral"),
    Curious         UMETA(DisplayName = "Curious"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Enlightened     UMETA(DisplayName = "Enlightened"),
    Corrupted       UMETA(DisplayName = "Corrupted")
};

USTRUCT(BlueprintType)
struct FConsciousnessThresholds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HostileThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NeutralThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriousThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FriendlyThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnlightenedThreshold;

    FConsciousnessThresholds()
    {
        HostileThreshold = 0.0f;
        NeutralThreshold = 0.2f;
        CuriousThreshold = 0.4f;
        FriendlyThreshold = 0.6f;
        EnlightenedThreshold = 0.8f;
    }
};

USTRUCT(BlueprintType)
struct FEnemyAIStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConsciousnessStability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriosityLevel;

    FEnemyAIStats()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        ConsciousnessLevel = 0.3f;
        ConsciousnessStability = 0.5f;
        AggressionLevel = 0.4f;
        FearLevel = 0.2f;
        CuriosityLevel = 0.3f;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAIStateChanged, EEnemyAIState, OldState, EEnemyAIState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessResponseChanged, EConsciousnessResponse, OldResponse, EConsciousnessResponse, NewResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDetected, AActor*, Player);

UCLASS()
class TRANSPERSONALGAME_API AEnemyAIController : public AAIController
{
    GENERATED_BODY()

public:
    AEnemyAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // AI State Management
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void SetAIState(EEnemyAIState NewState);

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    EEnemyAIState GetAIState() const { return CurrentAIState; }

    // Consciousness Response System
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void UpdateConsciousnessResponse(float PlayerConsciousnessLevel);

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    EConsciousnessResponse GetConsciousnessResponse() const { return CurrentConsciousnessResponse; }

    // Combat AI
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void InitiateCombat(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ExecuteBestAttack();

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    bool ShouldRetreat();

    // Perception Integration
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void OnPlayerPerceived(AActor* Actor, FAIStimulus Stimulus);

    // Consciousness Integration
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ModifyConsciousness(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void ReactToConsciousnessAttack(EConsciousnessAttackType AttackType, float Impact);

    // Adaptive Behavior
    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void AdaptBehaviorToPlayer(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Enemy AI")
    void UpdateEmotionalState(float DeltaTime);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnAIStateChanged OnAIStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnConsciousnessResponseChanged OnConsciousnessResponseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Enemy AI")
    FOnPlayerDetected OnPlayerDetected;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    FEnemyAIStats GetAIStats() const { return AIStats; }

    UFUNCTION(BlueprintPure, Category = "Enemy AI")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    // AI Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    class UBlackboardAsset* BlackboardAsset;

    // AI State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    EEnemyAIState CurrentAIState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    EConsciousnessResponse CurrentConsciousnessResponse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI State")
    FEnemyAIStats AIStats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Configuration")
    FConsciousnessThresholds ConsciousnessThresholds;

    // Combat
    UPROPERTY()
    AActor* CurrentTarget;

    UPROPERTY()
    class UTranspersonalCombatSystem* CombatSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float RetreatHealthThreshold;

    // Perception
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngleDegrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius;

    // Adaptive Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Behavior")
    float ConsciousnessAdaptationRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Behavior")
    float EmotionalDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Behavior")
    float StabilityRecoveryRate;

    // Timers
    UPROPERTY()
    float LastAttackTime;

    UPROPERTY()
    float LastConsciousnessCheck;

    UPROPERTY()
    float StateChangeTime;

private:
    void InitializePerception();
    void InitializeBehaviorTree();
    void UpdateBlackboard();
    EConsciousnessResponse DetermineConsciousnessResponse(float PlayerConsciousness);
    void HandleStateTransition(EEnemyAIState NewState);
    void UpdateCombatBehavior(float DeltaTime);
    void UpdateConsciousnessInfluence(float DeltaTime);
    FConsciousnessAttack SelectBestAttack();
    bool IsTargetInRange(float Range) const;
    void LogStateChange(EEnemyAIState OldState, EEnemyAIState NewState);
};