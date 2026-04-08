#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameplayTagContainer.h"
#include "TacticalCombatSystem.generated.h"

class ADinosaurCombatAI;
class UEnvironmentalQueryContext;

UENUM(BlueprintType)
enum class ETacticalRole : uint8
{
    None,
    Alpha,          // Líder do grupo, toma decisões táticas
    Flanker,        // Ataca pelos flancos
    Distractor,     // Chama atenção, cria diversões
    Ambusher,       // Ataca de emboscada
    Supporter       // Apoia outros dinossauros
};

UENUM(BlueprintType)
enum class ETacticalFormation : uint8
{
    None,
    PincerMovement,     // Movimento de pinça
    Encirclement,       // Cerco
    WaveAssault,        // Ataques em ondas
    AmbushSetup,        // Preparação de emboscada
    TerritoriaDefense   // Defesa territorial
};

USTRUCT(BlueprintType)
struct FTacticalPosition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadWrite)
    ETacticalRole AssignedRole = ETacticalRole::None;

    UPROPERTY(BlueprintReadWrite)
    float Priority = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsOccupied = false;

    UPROPERTY(BlueprintReadWrite)
    AActor* OccupyingActor = nullptr;

    FTacticalPosition()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        AssignedRole = ETacticalRole::None;
        Priority = 0.0f;
        bIsOccupied = false;
        OccupyingActor = nullptr;
    }
};

USTRUCT(BlueprintType)
struct FTacticalPlan
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    ETacticalFormation Formation = ETacticalFormation::None;

    UPROPERTY(BlueprintReadWrite)
    TArray<FTacticalPosition> Positions;

    UPROPERTY(BlueprintReadWrite)
    AActor* PrimaryTarget = nullptr;

    UPROPERTY(BlueprintReadWrite)
    float PlanConfidence = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    bool bIsActive = false;

    FTacticalPlan()
    {
        Formation = ETacticalFormation::None;
        Positions.Empty();
        PrimaryTarget = nullptr;
        PlanConfidence = 0.0f;
        ExecutionTime = 0.0f;
        bIsActive = false;
    }
};

/**
 * Sistema tático avançado que coordena múltiplos dinossauros em combate
 * Implementa formações, flanqueamento, emboscadas e coordenação de grupo
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATacticalCombatSystem : public AActor
{
    GENERATED_BODY()

public:
    ATacticalCombatSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Dinossauros registrados no sistema
    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    TArray<ADinosaurCombatAI*> RegisteredDinosaurs;

    // Plano tático atual
    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    FTacticalPlan CurrentPlan;

    // Histórico de planos para aprendizado
    UPROPERTY(BlueprintReadOnly, Category = "Tactical")
    TArray<FTacticalPlan> PlanHistory;

    // Configurações do sistema
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float PlanUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MaxCoordinationDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    int32 MaxCoordinatedDinosaurs = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MinPlanConfidence = 0.6f;

    // Timers
    float LastPlanUpdate = 0.0f;
    float PlanExecutionTimer = 0.0f;

public:
    // Registro de dinossauros
    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void RegisterDinosaur(ADinosaurCombatAI* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void UnregisterDinosaur(ADinosaurCombatAI* Dinosaur);

    // Planejamento tático
    UFUNCTION(BlueprintCallable, Category = "Tactical")
    bool CreateTacticalPlan(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void ExecuteTacticalPlan();

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void AbortCurrentPlan();

    // Coordenação em tempo real
    UFUNCTION(BlueprintCallable, Category = "Tactical")
    FVector GetOptimalFlankingPosition(AActor* Target, ADinosaurCombatAI* Flanker);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    bool ShouldInitiateGroupAttack(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    ETacticalRole AssignOptimalRole(ADinosaurCombatAI* Dinosaur, AActor* Target);

    // Comunicação entre dinossauros
    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void BroadcastThreatAlert(AActor* Threat, FVector ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void BroadcastAttackSignal(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Tactical")
    void BroadcastRetreatSignal();

    // Análise tática
    UFUNCTION(BlueprintPure, Category = "Tactical")
    float AnalyzeTacticalAdvantage(AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Tactical")
    bool CanExecuteFormation(ETacticalFormation Formation, AActor* Target);

    UFUNCTION(BlueprintPure, Category = "Tactical")
    TArray<FVector> GetFormationPositions(ETacticalFormation Formation, FVector CenterPoint, AActor* Target);

protected:
    // Lógica interna
    void UpdateTacticalSituation();
    void EvaluateCurrentPlan();
    void AdaptPlanBasedOnResults();
    
    // Formações específicas
    TArray<FVector> CalculatePincerPositions(FVector TargetLocation, FVector TargetDirection);
    TArray<FVector> CalculateEncirclementPositions(FVector TargetLocation, float Radius);
    TArray<FVector> CalculateAmbushPositions(FVector TargetLocation, FVector ApproachDirection);
    
    // Análise de terreno
    bool IsPositionSuitable(FVector Position, ETacticalRole Role);
    float GetTerrainAdvantage(FVector Position, AActor* Target);
    bool HasClearLineOfSight(FVector FromPosition, AActor* Target);
    
    // Aprendizado tático
    void RecordPlanOutcome(bool bSuccess, float EffectivenessScore);
    float GetFormationSuccessRate(ETacticalFormation Formation);
    void AdjustTacticsBasedOnHistory();

    // Utilitários
    ADinosaurCombatAI* FindAlphaDinosaur();
    TArray<ADinosaurCombatAI*> GetNearbyDinosaurs(FVector Location, float Radius);
    bool AreAlliesInPosition();
    float CalculateGroupCohesion();
};