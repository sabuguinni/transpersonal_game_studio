#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "CombatAITypes.h"
#include "DinosaurCombatController.generated.h"

class UBehaviorTree;
class UBlackboardAsset;

/**
 * Controller especializado para IA de combate de dinossauros
 * Implementa comportamentos táticos avançados e percepção sensorial
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

    // Componentes principais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UAIPerceptionComponent* AIPerceptionComponent;

    // Configuração de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatBehaviorConfig CombatConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBlackboardAsset* CombatBlackboard;

    // Estado atual de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    EDinosaurAlertState CurrentAlertState;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    AActor* CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    float LastAttackTime;

    UPROPERTY(BlueprintReadOnly, Category = "Combat State")
    bool bIsInCombat;

    // Configuração de percepção
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float LoseSightRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float FieldOfViewAngle = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange = 1200.0f;

public:
    // Funções públicas para Behavior Tree
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetAlertState(EDinosaurAlertState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanAttackTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FDinosaurAttack GetBestAttackForRange(float Range);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteAttack(const FDinosaurAttack& Attack);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldFlee();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CallForHelp();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalCombatPosition(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool IsTargetInOptimalRange(AActor* Target);

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Lógica interna
    void InitializePerception();
    void UpdateCombatState(float DeltaTime);
    void EvaluateThreatLevel(AActor* Actor);
    void UpdateBlackboardValues();
    
    // Sistema de cooldowns
    TMap<EDinosaurAttackType, float> AttackCooldowns;
    void UpdateAttackCooldowns(float DeltaTime);
    bool IsAttackReady(EDinosaurAttackType AttackType);

    // Sistema de memória tática
    struct FTacticalMemory
    {
        AActor* Actor;
        FVector LastKnownLocation;
        float LastSeenTime;
        float ThreatLevel;
        bool bIsHostile;
    };
    
    TArray<FTacticalMemory> TacticalMemoryList;
    void UpdateTacticalMemory(AActor* Actor, const FVector& Location, bool bIsHostile);
    FTacticalMemory* GetTacticalMemory(AActor* Actor);
};