#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameplayTagContainer.h"
#include "CombatBehaviorComponent.generated.h"

class ACombatAIManager;
class ADinosaurCharacter;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Alert           UMETA(DisplayName = "Alert"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    PackHunting     UMETA(DisplayName = "Pack Hunting"),
    Territorial     UMETA(DisplayName = "Territorial")
};

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator    UMETA(DisplayName = "Apex Predator"),     // T-Rex, Giganotosaurus
    PackHunter      UMETA(DisplayName = "Pack Hunter"),       // Velociraptor, Utahraptor
    Ambusher        UMETA(DisplayName = "Ambusher"),          // Carnotaurus, Baryonyx
    Herbivore       UMETA(DisplayName = "Herbivore"),         // Triceratops, Stegosaurus
    SmallHerbivore  UMETA(DisplayName = "Small Herbivore"),   // Parasaurolophus, Gallimimus
    Flying          UMETA(DisplayName = "Flying"),            // Pteranodon, Quetzalcoatlus
    Aquatic         UMETA(DisplayName = "Aquatic")            // Mosasaurus, Plesiosaur
};

USTRUCT(BlueprintType)
struct FCombatParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float FleeThreshold = 30.0f; // Percentual de vida para fugir

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Aggression = 50.0f; // 0-100

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float TerritorialRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bCanFormPacks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 MaxPackSize = 6;

    FCombatParameters()
    {
        AttackRange = 200.0f;
        DetectionRange = 1000.0f;
        FleeThreshold = 30.0f;
        Aggression = 50.0f;
        TerritorialRadius = 500.0f;
        bCanFormPacks = false;
        MaxPackSize = 6;
    }
};

/**
 * Componente de comportamento de combate para dinossauros
 * Gerencia estados de combate, coordenação tática e comportamentos emergentes
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado de Combate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombatState CurrentCombatState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    EDinosaurArchetype DinosaurArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Configuration")
    FCombatParameters CombatParams;

    // Referências
    UPROPERTY()
    ACombatAIManager* CombatManager;

    UPROPERTY()
    AAIController* OwnerAIController;

    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;

    // Alvos e Ameaças
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targets")
    AActor* CurrentTarget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targets")
    TArray<AActor*> KnownThreats;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targets")
    FVector LastKnownTargetLocation;

    // Sistema de Matilha
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    ADinosaurCharacter* PackLeader;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    TArray<ADinosaurCharacter*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Behavior")
    bool bIsPackLeader;

    // Timers e Cooldowns
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
    float LastAttackTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float AttackCooldown = 2.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Timing")
    float StateChangeTime;

    // Gameplay Tags para comportamentos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Tags")
    FGameplayTagContainer CombatTags;

public:
    // Interface Principal
    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SetCombatManager(ACombatAIManager* Manager);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat Behavior")
    ECombatState GetCombatState() const { return CurrentCombatState; }

    // Sistema de Alvos
    UFUNCTION(BlueprintCallable, Category = "Combat Targets")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat Targets")
    AActor* GetCurrentTarget() const { return CurrentTarget; }

    UFUNCTION(BlueprintCallable, Category = "Combat Targets")
    void AddThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat Targets")
    void RemoveThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Combat Targets")
    AActor* GetHighestPriorityThreat();

    // Sistema de Alertas
    UFUNCTION(BlueprintCallable, Category = "Combat Communication")
    void ReceiveAlert(AActor* ThreatActor, FVector AlertLocation, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Combat Communication")
    void BroadcastAlert(AActor* ThreatActor);

    // Sistema de Matilha
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void JoinPackHunt(ADinosaurCharacter* Leader, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void LeavePackHunt();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void BecomePackLeader();

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    bool CanFormPack() const;

    // Ações de Combate
    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    bool CanAttack() const;

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void ExecuteAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void RequestRepositioning();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    FVector GetOptimalAttackPosition(AActor* Target);

    // Comportamentos Específicos por Arquétipo
    UFUNCTION(BlueprintCallable, Category = "Archetype Behaviors")
    void ExecuteApexPredatorBehavior();

    UFUNCTION(BlueprintCallable, Category = "Archetype Behaviors")
    void ExecutePackHunterBehavior();

    UFUNCTION(BlueprintCallable, Category = "Archetype Behaviors")
    void ExecuteAmbusherBehavior();

    UFUNCTION(BlueprintCallable, Category = "Archetype Behaviors")
    void ExecuteHerbivoreBehavior();

    // Sistema de Avaliação de Ameaças
    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    float EvaluateThreatLevel(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    bool ShouldFlee(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Threat Assessment")
    bool ShouldEngage(AActor* ThreatActor);

protected:
    // Métodos internos de processamento
    void UpdateCombatState(float DeltaTime);
    void ProcessThreatAssessment();
    void UpdatePackCoordination();
    void HandleStateTransitions();

    // Métodos de comportamento por estado
    void ProcessPassiveState();
    void ProcessAlertState();
    void ProcessInvestigatingState();
    void ProcessStalkingState();
    void ProcessAttackingState();
    void ProcessFleeingState();
    void ProcessPackHuntingState();
    void ProcessTerritorialState();

    // Utilitários
    float CalculateDistanceToTarget() const;
    bool IsTargetInAttackRange() const;
    bool HasLineOfSightToTarget() const;
    void UpdateBlackboardValues();

    // Cache para otimização
    float LastThreatAssessmentTime;
    float ThreatAssessmentInterval = 0.5f;
};