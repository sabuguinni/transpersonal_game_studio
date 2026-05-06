#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "SharedTypes.h"
#include "Combat_TacticalAIController.generated.h"

UENUM(BlueprintType)
enum class ECombat_TacticalState : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Stalking        UMETA(DisplayName = "Stalking"),
    Flanking        UMETA(DisplayName = "Flanking"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Retreating      UMETA(DisplayName = "Retreating"),
    Coordinating    UMETA(DisplayName = "Coordinating")
};

UENUM(BlueprintType)
enum class ECombat_DinosaurRole : uint8
{
    SoloHunter      UMETA(DisplayName = "Solo Hunter"),
    PackLeader      UMETA(DisplayName = "Pack Leader"),
    PackMember      UMETA(DisplayName = "Pack Member"),
    Herbivore       UMETA(DisplayName = "Herbivore"),
    Scavenger       UMETA(DisplayName = "Scavenger")
};

USTRUCT(BlueprintType)
struct FCombat_TacticalData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PreferredAttackDistance = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float FlankingRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float RetreatHealthThreshold = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float CoordinationRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bUsesAmbushTactics = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    bool bCanCoordinateAttacks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical")
    float PatrolRadius = 2000.0f;

    FCombat_TacticalData()
    {
        PreferredAttackDistance = 300.0f;
        FlankingRadius = 800.0f;
        RetreatHealthThreshold = 25.0f;
        CoordinationRadius = 1500.0f;
        bUsesAmbushTactics = false;
        bCanCoordinateAttacks = false;
        PatrolRadius = 2000.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API ACombat_TacticalAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombat_TacticalAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Configuração da IA
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Setup")
    ECombat_DinosaurRole DinosaurRole = ECombat_DinosaurRole::SoloHunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI Setup")
    FCombat_TacticalData TacticalData;

    // Estado atual
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State")
    ECombat_TacticalState CurrentTacticalState = ECombat_TacticalState::Passive;

    // Componentes de IA
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UAIPerceptionComponent* AIPerceptionComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI Components")
    class UBehaviorTreeComponent* BehaviorTreeComponent;

    // Behavior Tree e Blackboard
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Assets")
    class UBlackboardAsset* BlackboardAsset;

    // Alvo atual e dados de combate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targeting")
    AActor* CurrentTarget = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targeting")
    FVector LastKnownTargetLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat Targeting")
    float TimeSinceLastTargetSight = 0.0f;

    // Coordenação de grupo
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    TArray<ACombat_TacticalAIController*> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    ACombat_TacticalAIController* PackLeader = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pack Coordination")
    bool bIsPackLeader = false;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTacticalState(ECombat_TacticalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector CalculateFlankingPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCoordinatedAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void JoinPack(ACombat_TacticalAIController* Leader);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LeavePack();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat() const;

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector GetOptimalAttackPosition() const;

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Funções de estado tático
    void UpdatePassiveState(float DeltaTime);
    void UpdateInvestigatingState(float DeltaTime);
    void UpdateStalkingState(float DeltaTime);
    void UpdateFlankingState(float DeltaTime);
    void UpdateAttackingState(float DeltaTime);
    void UpdateRetreatingState(float DeltaTime);
    void UpdateCoordinatingState(float DeltaTime);

    // Funções de coordenação
    void FindNearbyPackMembers();
    void SendPackCommand(const FString& Command);
    void ReceivePackCommand(const FString& Command, ACombat_TacticalAIController* Sender);

    // Funções de análise tática
    bool IsTargetInOptimalRange() const;
    bool HasClearPathToTarget() const;
    FVector CalculateAmbushPosition() const;
    bool ShouldInitiateAttack() const;

    // Configuração por espécie
    void ConfigureForSpecies();
    void ConfigureTRexTactics();
    void ConfigureRaptorTactics();
    void ConfigureBrachioTactics();

    // Variáveis internas
    float StateTimer = 0.0f;
    float LastCoordinationUpdate = 0.0f;
    FVector PatrolCenter;
    bool bHasInitialized = false;
};

#include "Combat_TacticalAIController.generated.h"