#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../Combat/CombatSystem.h"
#include "../Core/ConsciousnessComponent.h"
#include "EnemyAI.generated.h"

UENUM(BlueprintType)
enum class EEnemyPersonality : uint8
{
    Aggressive     UMETA(DisplayName = "Aggressive"),
    Defensive      UMETA(DisplayName = "Defensive"),
    Tactical       UMETA(DisplayName = "Tactical"),
    Spiritual      UMETA(DisplayName = "Spiritual"),
    Chaotic        UMETA(DisplayName = "Chaotic")
};

UENUM(BlueprintType)
enum class EConsciousnessArchetype : uint8
{
    Shadow         UMETA(DisplayName = "Shadow - Low Consciousness"),
    Ego            UMETA(DisplayName = "Ego - Medium Consciousness"),
    Soul           UMETA(DisplayName = "Soul - High Consciousness"),
    Spirit         UMETA(DisplayName = "Spirit - Transcendent")
};

USTRUCT(BlueprintType)
struct FEnemyProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnemyPersonality Personality = EEnemyPersonality::Aggressive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessArchetype Archetype = EConsciousnessArchetype::Shadow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseConsciousnessLevel = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntelligenceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpiritualAwareness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AdaptabilityRate = 1.0f;

    FEnemyProfile()
    {
        Personality = EEnemyPersonality::Aggressive;
        Archetype = EConsciousnessArchetype::Shadow;
        BaseConsciousnessLevel = 25.0f;
        AggressionLevel = 0.7f;
        IntelligenceLevel = 0.5f;
        SpiritualAwareness = 0.3f;
        AdaptabilityRate = 1.0f;
    }
};

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
    // AI Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Enemy Profile
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy Profile")
    FEnemyProfile EnemyProfile;

    // Behavior Trees for different archetypes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* ShadowBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* EgoBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* SoulBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* SpiritBehaviorTree;

    // Component References
    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UCombatSystem* CombatSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    UConsciousnessComponent* ConsciousnessComponent;

    // AI State
    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    FVector LastKnownTargetLocation;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    float TimeSinceLastTargetSeen = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "AI State")
    bool bIsInCombat = false;

    // Consciousness Evolution
    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    float ConsciousnessEvolutionRate = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Consciousness")
    float LastConsciousnessLevel = 0.0f;

    // AI Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetEnemyProfile(const FEnemyProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void OnTargetPerceived(AActor* Actor, FAIStimulus Stimulus);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void OnTargetLost();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateConsciousnessBasedBehavior(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void AdaptToPlayerBehavior();

    // Consciousness-Based Decision Making
    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldEngage() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool ShouldSeekDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    float GetOptimalAttackDistance() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    EAttackType GetPreferredAttackType() const;

    // Archetype-Specific Behaviors
    UFUNCTION(BlueprintCallable, Category = "AI")
    void ExecuteShadowBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ExecuteEgoBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ExecuteSoulBehavior();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void ExecuteSpiritBehavior();

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "AI")
    void OnArchetypeChanged(EConsciousnessArchetype OldArchetype, EConsciousnessArchetype NewArchetype);

    UFUNCTION(BlueprintImplementableEvent, Category = "AI")
    void OnConsciousnessEvolution(float NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "AI")
    void OnBehaviorAdaptation();

private:
    void InitializePerception();
    void UpdateArchetypeBasedOnConsciousness();
    void SetBlackboardValues();
    EConsciousnessArchetype DetermineArchetypeFromConsciousness(float ConsciousnessLevel) const;
    UBehaviorTree* GetBehaviorTreeForArchetype(EConsciousnessArchetype Archetype) const;
};