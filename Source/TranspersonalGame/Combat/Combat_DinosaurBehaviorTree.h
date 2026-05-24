#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Tasks/BTTaskNode.h"
#include "BehaviorTree/Decorators/BTDecorator.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "NavigationSystem.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Combat_DinosaurBehaviorTree.generated.h"

// Enum para estados de combate dos dinossauros
UENUM(BlueprintType)
enum class ECombat_DinosaurCombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"), 
    Investigating UMETA(DisplayName = "Investigating"),
    Stalking    UMETA(DisplayName = "Stalking"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Feeding     UMETA(DisplayName = "Feeding"),
    Resting     UMETA(DisplayName = "Resting")
};

// Enum para tipos de dinossauros
UENUM(BlueprintType)
enum class ECombat_DinosaurType : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Raptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus")
};

// Struct para dados de combate do dinossauro
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombat_DinosaurCombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    ECombat_DinosaurType DinosaurType = ECombat_DinosaurType::Raptor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float MovementSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AggressionLevel = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsHerbivore = false;

    FCombat_DinosaurCombatData()
    {
        DinosaurType = ECombat_DinosaurType::Raptor;
        Health = 100.0f;
        MaxHealth = 100.0f;
        AttackDamage = 25.0f;
        AttackRange = 200.0f;
        DetectionRange = 1500.0f;
        MovementSpeed = 600.0f;
        AggressionLevel = 0.7f;
        bIsPackHunter = false;
        bIsHerbivore = false;
    }
};

// Task Node para atacar o jogador
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_AttackPlayer : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_AttackPlayer();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackDamage = 25.0f;

    UPROPERTY(EditAnywhere, Category = "Combat")
    float AttackRange = 200.0f;
};

// Task Node para patrulhar área
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_PatrolArea : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_PatrolArea();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float WaitTime = 3.0f;
};

// Task Node para comunicação entre dinossauros do mesmo pack
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTTask_PackCommunication : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UCombat_BTTask_PackCommunication();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Pack")
    float CommunicationRange = 2000.0f;

    UPROPERTY(EditAnywhere, Category = "Pack")
    bool bAlertNearbyPack = true;
};

// Decorator para verificar se o jogador está no alcance
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTDecorator_PlayerInRange : public UBTDecorator
{
    GENERATED_BODY()

public:
    UCombat_BTDecorator_PlayerInRange();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Detection")
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "Detection")
    bool bRequireLineOfSight = true;
};

// Decorator para verificar saúde do dinossauro
UCLASS()
class TRANSPERSONALGAME_API UCombat_BTDecorator_HealthCheck : public UBTDecorator
{
    GENERATED_BODY()

public:
    UCombat_BTDecorator_HealthCheck();

    virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

protected:
    UPROPERTY(EditAnywhere, Category = "Health")
    float HealthThreshold = 30.0f;

    UPROPERTY(EditAnywhere, Category = "Health")
    bool bCheckLowHealth = true;
};

// Componente principal para comportamento de dinossauros
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombat_DinosaurBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombat_DinosaurBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurar comportamento baseado no tipo de dinossauro
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetupDinosaurBehavior(ECombat_DinosaurType InDinosaurType);

    // Detectar jogador próximo
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool DetectPlayer(float DetectionRadius = 1500.0f);

    // Atacar jogador
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AttackPlayer(float Damage = 25.0f);

    // Comunicar com outros dinossauros
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AlertNearbyDinosaurs(float AlertRadius = 2000.0f);

    // Fugir quando com pouca saúde
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void FleeFromPlayer();

    // Patrulhar área
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void PatrolArea(float PatrolRadius = 1000.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Data")
    FCombat_DinosaurCombatData CombatData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
    ECombat_DinosaurCombatState CurrentState = ECombat_DinosaurCombatState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Tree")
    UBehaviorTree* DinosaurBehaviorTree;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPawnSensingComponent* PawnSensingComponent;

    // Timer para patrulha
    FTimerHandle PatrolTimerHandle;

    // Última posição conhecida do jogador
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FVector LastKnownPlayerLocation;

    // Referência para o jogador detectado
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    APawn* DetectedPlayer;

private:
    // Configurações específicas por tipo de dinossauro
    void ConfigureTRexBehavior();
    void ConfigureRaptorBehavior();
    void ConfigureHerbivoreBehavior();

    // Funções de callback para detecção
    UFUNCTION()
    void OnPlayerDetected(APawn* DetectedPawn);

    UFUNCTION()
    void OnPlayerLost(APawn* LostPawn);
};