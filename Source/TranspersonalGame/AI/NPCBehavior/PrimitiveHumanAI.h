#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "../NPCBehaviorTypes.h"
#include "PrimitiveHumanAI.generated.h"

class UBehaviorTree;
class UBlackboardData;

/**
 * AI Controller for primitive human NPCs in the prehistoric world
 * Handles survival behaviors: gathering, fleeing from dinosaurs, social interactions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPC_PrimitiveHumanAI : public AAIController
{
    GENERATED_BODY()

public:
    ANPC_PrimitiveHumanAI();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Behavior Tree and Blackboard
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBehaviorTree* BehaviorTree;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UBlackboardData* BlackboardAsset;

    // AI Perception
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    UAISenseConfig_Hearing* HearingConfig;

    // Behavior State
    UPROPERTY(BlueprintReadWrite, Category = "AI")
    ENPC_BehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadWrite, Category = "AI")
    ENPC_EmotionalState EmotionalState;

    // Survival Stats
    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Hunger;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Thirst;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Fear;

    UPROPERTY(BlueprintReadWrite, Category = "Survival")
    float Energy;

    // Social Behavior
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    TArray<AActor*> NearbyHumans;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    AActor* CurrentLeader;

    UPROPERTY(BlueprintReadWrite, Category = "Social")
    bool bIsGroupLeader;

    // Threat Detection
    UPROPERTY(BlueprintReadWrite, Category = "Threats")
    TArray<AActor*> DetectedThreats;

    UPROPERTY(BlueprintReadWrite, Category = "Threats")
    AActor* PrimaryThreat;

    UPROPERTY(BlueprintReadWrite, Category = "Threats")
    float ThreatLevel;

    // Gathering Behavior
    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    TArray<AActor*> NearbyResources;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    AActor* TargetResource;

    UPROPERTY(BlueprintReadWrite, Category = "Gathering")
    FVector LastGatherLocation;

public:
    // Perception Events
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Behavior State Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void EvaluateThreats();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void FindNearbyResources();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateSocialBehavior();

    // Blackboard Key Management
    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardKeys();

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool NeedsFood() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool NeedsWater() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    bool IsExhausted() const;

    UFUNCTION(BlueprintCallable, Category = "AI")
    FVector GetSafeLocation() const;

private:
    // Internal timers
    float SurvivalUpdateTimer;
    float ThreatEvaluationTimer;
    float SocialUpdateTimer;

    // Constants
    static constexpr float SURVIVAL_UPDATE_INTERVAL = 1.0f;
    static constexpr float THREAT_EVALUATION_INTERVAL = 0.5f;
    static constexpr float SOCIAL_UPDATE_INTERVAL = 2.0f;
    static constexpr float DANGER_THRESHOLD = 0.7f;
    static constexpr float HUNGER_THRESHOLD = 0.6f;
    static constexpr float THIRST_THRESHOLD = 0.7f;
    static constexpr float ENERGY_THRESHOLD = 0.3f;
};