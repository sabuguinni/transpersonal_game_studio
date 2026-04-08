#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags.h"
#include "CombatAISystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive,        // Herbívoros pequenos - domesticáveis
    Defensive,      // Herbívoros grandes - defendem território
    Predator,       // Carnívoros pequenos/médios - caçam em grupo
    ApexPredator    // Carnívoros grandes - caçam sozinhos
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,
    Hunting,
    Stalking,
    Attacking,
    Fleeing,
    Territorial,
    Feeding,
    Resting
};

UENUM(BlueprintType)
enum class EHuntingStrategy : uint8
{
    Ambush,         // Emboscada - esperar escondido
    Pack,           // Matilha - coordenar com outros
    Persistence,    // Persistência - perseguir até cansar
    Territorial,    // Territorial - defender área
    Opportunistic   // Oportunista - atacar quando vantajoso
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPersonality
{
    GENERATED_BODY()

    // Agressividade base (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    // Inteligência tática (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Paciência para emboscadas (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Patience = 0.5f;

    // Tendência para trabalho em grupo (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Cooperation = 0.5f;

    // Medo/cautela (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Caution = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatCapabilities
{
    GENERATED_BODY()

    // Velocidade máxima
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxSpeed = 600.0f;

    // Raio de detecção
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRadius = 1500.0f;

    // Raio de ataque
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    // Dano base
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 25.0f;

    // Resistência (stamina)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Stamina = 100.0f;

    // Pode trabalhar em grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanPackHunt = false;

    // Pode usar emboscadas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanAmbush = false;

    // Estratégia preferida
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EHuntingStrategy PreferredStrategy = EHuntingStrategy::Opportunistic;
};

/**
 * Sistema central de IA de combate para dinossauros
 * Gere comportamentos táticos inteligentes baseados em personalidade e capacidades
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAISystem : public AActor
{
    GENERATED_BODY()

public:
    ACombatAISystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Registra um dinossauro no sistema de combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void RegisterCombatant(class ADinosaurAI* Dinosaur);

    // Remove um dinossauro do sistema
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UnregisterCombatant(class ADinosaurAI* Dinosaur);

    // Encontra inimigos próximos para um dinossauro
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyEnemies(class ADinosaurAI* Dinosaur, float SearchRadius = 2000.0f);

    // Coordena ataque em grupo
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinatePackAttack(const TArray<class ADinosaurAI*>& PackMembers, AActor* Target);

    // Avalia se deve atacar baseado em vantagem tática
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldInitiateAttack(class ADinosaurAI* Attacker, AActor* Target);

    // Encontra posição de emboscada
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    FVector FindAmbushPosition(class ADinosaurAI* Dinosaur, AActor* Target);

protected:
    // Lista de todos os combatentes registados
    UPROPERTY()
    TArray<class ADinosaurAI*> RegisteredCombatants;

    // Grupos de caça activos
    UPROPERTY()
    TMap<FString, TArray<class ADinosaurAI*>> ActiveHuntingPacks;

    // Sistema de coordenação de ataques
    UFUNCTION()
    void UpdatePackCoordination(float DeltaTime);

    // Calcula vantagem tática
    UFUNCTION()
    float CalculateTacticalAdvantage(class ADinosaurAI* Attacker, AActor* Target);
};

/**
 * Controlador de IA especializado para combate de dinossauros
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurCombatController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurCombatController();

protected:
    virtual void BeginPlay() override;

public:
    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UAIPerceptionComponent* AIPerceptionComponent;

    // Blackboard para dados de combate
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBlackboardAsset* CombatBlackboard;

    // Behavior Tree de combate
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
    class UBehaviorTree* CombatBehaviorTree;

    // Personalidade de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatPersonality CombatPersonality;

    // Capacidades de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FCombatCapabilities CombatCapabilities;

    // Nível de ameaça
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EDinosaurThreatLevel ThreatLevel = EDinosaurThreatLevel::Defensive;

    // Estado actual de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    ECombatState CurrentCombatState = ECombatState::Idle;

    // Alvo actual
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* CurrentTarget = nullptr;

    // Membros do grupo (se aplicável)
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    TArray<class ADinosaurAI*> PackMembers;

    // Funções de combate
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    float GetDistanceToTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void JoinPack(const TArray<class ADinosaurAI*>& NewPackMembers);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void LeavePack();

protected:
    // Callbacks de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // Lógica de tomada de decisão
    UFUNCTION()
    void EvaluateThreatLevel();

    UFUNCTION()
    void UpdateCombatBehavior(float DeltaTime);
};