#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"

/**
 * COMBAT AI ARCHITECTURE
 * Transpersonal Game Studio - Jurassic Survival Game
 * 
 * Sistema de IA de combate baseado em:
 * - Behavior Trees para decisões táticas
 * - AI Perception para detecção de ameaças
 * - EQS para posicionamento tático
 * - Gameplay Tags para estados de combate
 * - State Trees para comportamentos complexos
 */

UENUM(BlueprintType)
enum class EDinosaurCombatRole : uint8
{
    Apex_Predator,      // T-Rex, Giganotosaurus - Dominam território
    Pack_Hunter,        // Velociraptor, Deinonychus - Caçam em grupo
    Ambush_Predator,    // Carnotaurus, Baryonyx - Ataques surpresa
    Territorial_Guard,  // Triceratops, Ankylosaurus - Defendem área
    Opportunist,        // Compsognathus, Dilophosaurus - Aproveitam situações
    Passive_Herbivore   // Parasaurolophus, Brachiosaurus - Fogem do perigo
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle,               // Estado neutro
    Hunting,            // Procurando presa
    Stalking,           // Seguindo presa silenciosamente
    Engaging,           // Iniciando combate
    Fighting,           // Em combate ativo
    Retreating,         // Fugindo ou recuando
    Guarding,           // Protegendo território/recursos
    Feeding,            // Consumindo presa
    Wounded,            // Ferido, comportamento alterado
    Domesticated        // Domesticado pelo jogador
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None,               // Sem ameaça
    Low,                // Ameaça menor
    Medium,             // Ameaça moderada
    High,               // Ameaça alta
    Critical            // Ameaça mortal
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPersonality
{
    GENERATED_BODY()

    // Agressividade base (0.0 = pacífico, 1.0 = extremamente agressivo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;

    // Inteligência tática (0.0 = instintivo, 1.0 = muito inteligente)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Curiosidade (0.0 = evita novidades, 1.0 = investiga tudo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;

    // Territorialidade (0.0 = nômade, 1.0 = extremamente territorial)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    // Sociabilidade (0.0 = solitário, 1.0 = sempre em grupo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;

    // Coragem (0.0 = covarde, 1.0 = destemido)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Courage = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    // Última posição conhecida do jogador
    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    // Tempo desde que viu o jogador pela última vez
    UPROPERTY(BlueprintReadWrite)
    float TimeSincePlayerSeen = 0.0f;

    // Nível de alerta atual
    UPROPERTY(BlueprintReadWrite)
    EThreatLevel CurrentThreatLevel = EThreatLevel::None;

    // Locais onde o jogador foi visto (para patrulhamento)
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> PlayerSightings;

    // Dinossauros aliados conhecidos
    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownAllies;

    // Inimigos conhecidos
    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownEnemies;
};

/**
 * Interface para objetos que podem ser atacados
 */
UINTERFACE(BlueprintType)
class TRANSPERSONALGAME_API UCombatTarget : public UInterface
{
    GENERATED_BODY()
};

class TRANSPERSONALGAME_API ICombatTarget
{
    GENERATED_BODY()

public:
    // Retorna o nível de ameaça que este alvo representa
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    EThreatLevel GetThreatLevel() const;

    // Retorna se este alvo pode ser atacado
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    bool CanBeAttacked() const;

    // Chamado quando este alvo é atacado
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    void OnAttacked(AActor* Attacker, float Damage);

    // Retorna a prioridade deste alvo (maior = mais prioritário)
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
    float GetTargetPriority() const;
};

/**
 * Componente base para IA de combate
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCombatAIComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCombatAIComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    EDinosaurCombatRole CombatRole = EDinosaurCombatRole::Opportunist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatPersonality Personality;

    // Estado atual de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    ECombatState CurrentCombatState = ECombatState::Idle;

    // Memória de combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombatMemory CombatMemory;

    // Behavior Tree para este tipo de dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    UBehaviorTree* CombatBehaviorTree;

    // Blackboard para comunicação com Behavior Tree
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    UBlackboardComponent* CombatBlackboard;

    // Componente de percepção
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UAIPerceptionComponent* PerceptionComponent;

    // Tags de gameplay para estados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FGameplayTagContainer CombatTags;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    AActor* FindBestTarget();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool CanSeeTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetDistanceToTarget(AActor* Target);

    // Eventos de percepção
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // Funções de personalidade
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldEngageTarget(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float GetPersonalityModifier(const FString& ModifierName);

private:
    // Timer para atualização de estado
    FTimerHandle StateUpdateTimer;

    // Função para atualizar estado baseado em condições
    void UpdateCombatState();

    // Função para gerar personalidade única
    void GenerateUniquePersonality();
};