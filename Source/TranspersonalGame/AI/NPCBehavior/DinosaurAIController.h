#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "Perception/AIDamageConfig.h"
#include "Perception/AITouchConfig.h"
#include "../NPCBehaviorSystem.h"
#include "DinosaurAIController.generated.h"

class UNPCBehaviorComponent;
class UAdvancedNPCBehaviorComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurPerceptionUpdated, AActor*, Actor, FAIStimulus, Stimulus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDinosaurBehaviorChanged, EDinosaurBehaviorState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDinosaurRelationshipChanged, AActor*, Actor, float, NewRelationshipValue);

/**
 * Controlador AI especializado para dinossauros
 * Integra percepção avançada, tomada de decisão baseada em personalidade e memória
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

public:
    // === COMPONENTES PRINCIPAIS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBehaviorTreeComponent* BehaviorTreeComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UBlackboardComponent* BlackboardComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    UAIPerceptionComponent* AIPerceptionComponent;

    // === CONFIGURAÇÃO DE PERCEPÇÃO ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float SightRadius = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float LoseSightRadius = 2200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float PeripheralVisionAngleDegrees = 120.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float HearingRange = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Perception")
    float TouchRange = 100.0f;

    // === BEHAVIOR TREE MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StartBehaviorTree(UBehaviorTree* BehaviorTree);
    
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void StopBehaviorTree();
    
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    void RestartBehaviorTree();
    
    UFUNCTION(BlueprintCallable, Category = "AI Behavior")
    bool IsBehaviorTreeRunning() const;

    // === BLACKBOARD MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsObject(const FName& KeyName, UObject* ObjectValue);
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsVector(const FName& KeyName, FVector VectorValue);
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsBool(const FName& KeyName, bool BoolValue);
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsFloat(const FName& KeyName, float FloatValue);
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsInt(const FName& KeyName, int32 IntValue);
    
    UFUNCTION(BlueprintCallable, Category = "AI Blackboard")
    void SetBlackboardValueAsEnum(const FName& KeyName, uint8 EnumValue);

    // === PERCEPTION CALLBACKS ===
    
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);
    
    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
    
    UFUNCTION()
    void OnTargetPerceptionForgotten(AActor* Actor);

    // === DECISION MAKING ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    EDinosaurBehaviorState DecideBehaviorState();
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    AActor* FindMostImportantTarget();
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    FVector FindBestLocationForBehavior(EDinosaurBehaviorState BehaviorState);
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    float CalculateActorThreatLevel(AActor* Actor);
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    bool ShouldFleeFromActor(AActor* Actor);
    
    UFUNCTION(BlueprintCallable, Category = "AI Decision")
    bool ShouldInvestigateActor(AActor* Actor);

    // === SOCIAL BEHAVIOR ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    TArray<AActor*> FindNearbyDinosaurs(float Radius = 1000.0f);
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    TArray<AActor*> FindPackMembers();
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    AActor* FindPackLeader();
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    bool IsInPack() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    void JoinPack(AActor* PackLeader);
    
    UFUNCTION(BlueprintCallable, Category = "AI Social")
    void LeavePack();

    // === MEMORY INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void UpdateMemoryFromPerception();
    
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    void RememberImportantEvent(const FString& EventType, AActor* RelatedActor, float EmotionalWeight);
    
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    bool HasMemoryOfActor(AActor* Actor);
    
    UFUNCTION(BlueprintCallable, Category = "AI Memory")
    float GetMemoryThreatLevel(AActor* Actor);

    // === DOMESTICATION SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Domestication")
    void ProcessPlayerInteraction(bool bPositive, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "AI Domestication")
    bool CanBeDomesticated() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Domestication")
    float GetDomesticationProgress() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Domestication")
    EDomesticationLevel GetDomesticationLevel() const;

    // === ENVIRONMENTAL AWARENESS ===
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    FVector FindNearestWaterSource();
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    FVector FindNearestFoodSource();
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    FVector FindSafeRestingSpot();
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    bool IsLocationSafe(FVector Location);
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    float GetTimeOfDay() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Environment")
    bool IsPreferredActivityTime() const;

    // === EVENTOS ===
    
    UPROPERTY(BlueprintAssignable, Category = "AI Events")
    FOnDinosaurPerceptionUpdated OnPerceptionUpdatedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "AI Events")
    FOnDinosaurBehaviorChanged OnBehaviorChangedDelegate;
    
    UPROPERTY(BlueprintAssignable, Category = "AI Events")
    FOnDinosaurRelationshipChanged OnRelationshipChangedDelegate;

protected:
    // === COMPONENTES INTERNOS ===
    
    UPROPERTY()
    UNPCBehaviorComponent* NPCBehaviorComponent;
    
    UPROPERTY()
    UAdvancedNPCBehaviorComponent* AdvancedBehaviorComponent;

    // === CONFIGURAÇÃO DE PERCEPÇÃO ===
    
    UPROPERTY()
    UAISightConfig* SightConfig;
    
    UPROPERTY()
    UAIHearingConfig* HearingConfig;
    
    UPROPERTY()
    UAIDamageConfig* DamageConfig;
    
    UPROPERTY()
    UAITouchConfig* TouchConfig;

    // === ESTADO INTERNO ===
    
    UPROPERTY()
    EDinosaurBehaviorState LastBehaviorState;
    
    UPROPERTY()
    float LastDecisionTime;
    
    UPROPERTY()
    float DecisionInterval = 1.0f; // Intervalo entre decisões em segundos
    
    UPROPERTY()
    TArray<AActor*> CurrentlyPerceivedActors;
    
    UPROPERTY()
    AActor* CurrentTarget;
    
    UPROPERTY()
    FVector CurrentDestination;

    // === FUNÇÕES INTERNAS ===
    
    void InitializePerception();
    void InitializeBlackboard();
    void UpdateBlackboardValues();
    void ProcessPerceptionData();
    void MakeDecisions();
    void UpdateBehaviorState();
    void HandleStateTransition(EDinosaurBehaviorState NewState);
    
    // Funções de análise de percepção
    void AnalyzeThreatLevel();
    void AnalyzeOpportunities();
    void AnalyzeSocialSituation();
    
    // Funções de tomada de decisão baseada em personalidade
    bool ShouldReactToStimulus(const FAIStimulus& Stimulus);
    float CalculatePersonalityInfluence(const FString& DecisionType);
    bool PassesPersonalityCheck(const FString& TraitName, float Threshold);

public:
    // === BLACKBOARD KEYS (constantes) ===
    
    static const FName BB_CurrentTarget;
    static const FName BB_CurrentDestination;
    static const FName BB_BehaviorState;
    static const FName BB_IsPlayerNear;
    static const FName BB_PlayerLocation;
    static const FName BB_ThreatLevel;
    static const FName BB_NearestWater;
    static const FName BB_NearestFood;
    static const FName BB_SafeLocation;
    static const FName BB_PackMembers;
    static const FName BB_IsInPack;
    static const FName BB_PackLeader;
    static const FName BB_DomesticationLevel;
    static const FName BB_CurrentNeeds;
    static const FName BB_EmotionalState;
    static const FName BB_TimeOfDay;
    static const FName BB_IsPreferredTime;
};