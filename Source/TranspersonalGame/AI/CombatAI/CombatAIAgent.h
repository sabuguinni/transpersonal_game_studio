#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "EnvironmentalQuery/EnvQuery.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "CombatAIArchitecture.h"
#include "../DinosaurArchetypes.h"
#include "CombatAIAgent.generated.h"

/**
 * COMBAT AI AGENT - Agente #12
 * Transpersonal Game Studio
 * 
 * Sistema de inteligência artificial de combate para dinossauros
 * Baseado em Behavior Trees, AI Perception, EQS e Gameplay Tags
 * 
 * FILOSOFIA:
 * - Cada combate é uma conversa, não uma equação
 * - Inimigos que comentam o que vêem são mais aterrorizadores
 * - 30 segundos de gameplay divertido repetido com variação
 * - O jogador deve sempre sentir que pode ganhar até ao último segundo
 * - Morte por erro do jogador, não por injustiça do jogo
 */

UENUM(BlueprintType)
enum class ECombatPhase : uint8
{
    Detection,          // Detectando ameaças
    Assessment,         // Avaliando situação
    Positioning,        // Posicionamento tático
    Engagement,         // Combate ativo
    Adaptation,         // Adaptação durante combate
    Resolution          // Resolução (vitória/fuga/morte)
};

UENUM(BlueprintType)
enum class ETacticalBehavior : uint8
{
    Direct_Assault,     // Ataque direto e agressivo
    Ambush,            // Emboscada e ataque surpresa
    Hit_And_Run,       // Ataque rápido e recuo
    Territorial_Defense, // Defesa de território
    Pack_Coordination,  // Coordenação em grupo
    Intimidation,      // Intimidação sem combate
    Retreat,           // Retirada tática
    Stalking           // Seguir e esperar oportunidade
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatIntelligence
{
    GENERATED_BODY()

    // Capacidade de aprendizagem (0.0 = não aprende, 1.0 = aprende rapidamente)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float LearningRate = 0.3f;

    // Capacidade de adaptação durante combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AdaptabilityRate = 0.4f;

    // Capacidade de coordenação com outros dinossauros
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CoordinationSkill = 0.2f;

    // Capacidade de prever movimentos do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PredictionSkill = 0.1f;

    // Memória de padrões do jogador
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, float> PlayerPatterns;

    // Contador de encontros com o jogador
    UPROPERTY(BlueprintReadWrite)
    int32 EncounterCount = 0;

    FCombatIntelligence()
    {
        LearningRate = 0.3f;
        AdaptabilityRate = 0.4f;
        CoordinationSkill = 0.2f;
        PredictionSkill = 0.1f;
        EncounterCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatContext
{
    GENERATED_BODY()

    // Fase atual do combate
    UPROPERTY(BlueprintReadWrite)
    ECombatPhase CurrentPhase = ECombatPhase::Detection;

    // Comportamento tático atual
    UPROPERTY(BlueprintReadWrite)
    ETacticalBehavior CurrentTactic = ETacticalBehavior::Direct_Assault;

    // Alvo principal
    UPROPERTY(BlueprintReadWrite)
    AActor* PrimaryTarget = nullptr;

    // Alvos secundários
    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> SecondaryTargets;

    // Aliados próximos
    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> NearbyAllies;

    // Posição tática desejada
    UPROPERTY(BlueprintReadWrite)
    FVector TacticalPosition = FVector::ZeroVector;

    // Tempo no combate atual
    UPROPERTY(BlueprintReadWrite)
    float CombatDuration = 0.0f;

    // Saúde no início do combate (para calcular dano recebido)
    UPROPERTY(BlueprintReadWrite)
    float InitialHealth = 100.0f;

    // Nível de stress/pressão
    UPROPERTY(BlueprintReadWrite)
    float StressLevel = 0.0f;

    FCombatContext()
    {
        CurrentPhase = ECombatPhase::Detection;
        CurrentTactic = ETacticalBehavior::Direct_Assault;
        PrimaryTarget = nullptr;
        TacticalPosition = FVector::ZeroVector;
        CombatDuration = 0.0f;
        InitialHealth = 100.0f;
        StressLevel = 0.0f;
    }
};

/**
 * Combat AI Controller
 * Controlador de IA especializado em combate tático
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACombatAIController : public AAIController
{
    GENERATED_BODY()

public:
    ACombatAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;

public:
    // Configuração do dinossauro
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    EDinosaurCombatRole CombatRole = EDinosaurCombatRole::Opportunist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FDinosaurArchetype DinosaurArchetype;

    // Inteligência de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat AI")
    FCombatIntelligence CombatIntelligence;

    // Contexto atual do combate
    UPROPERTY(BlueprintReadOnly, Category = "Combat AI")
    FCombatContext CombatContext;

    // Behavior Trees por situação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    UBehaviorTree* IdleBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    UBehaviorTree* HuntingBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    UBehaviorTree* CombatBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior Trees")
    UBehaviorTree* FleeingBehaviorTree;

    // EQS Queries para posicionamento tático
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
    UEnvQuery* FindCoverQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
    UEnvQuery* FindFlankingPositionQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
    UEnvQuery* FindAmbushPositionQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EQS")
    UEnvQuery* FindRetreatPositionQuery;

    // Componente de combate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat AI")
    UCombatAIComponent* CombatComponent;

public:
    // === FUNÇÕES PRINCIPAIS ===

    // Iniciar combate com alvo específico
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void InitiateCombat(AActor* Target);

    // Terminar combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EndCombat(bool bVictorious);

    // Atualizar fase do combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void UpdateCombatPhase(ECombatPhase NewPhase);

    // Selecionar tática baseada na situação
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    ETacticalBehavior SelectOptimalTactic();

    // === SISTEMA DE APRENDIZAGEM ===

    // Aprender com o comportamento do jogador
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void LearnFromPlayerBehavior(const FString& BehaviorPattern, float Effectiveness);

    // Adaptar comportamento baseado na experiência
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void AdaptBehavior();

    // === COORDENAÇÃO DE GRUPO ===

    // Encontrar aliados próximos
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    TArray<AActor*> FindNearbyAllies(float SearchRadius = 1000.0f);

    // Coordenar ataque em grupo
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CoordinateGroupAttack(const TArray<AActor*>& Allies, AActor* Target);

    // Comunicar ameaça para outros dinossauros
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CommunicateThreat(AActor* Threat, EThreatLevel ThreatLevel);

    // === POSICIONAMENTO TÁTICO ===

    // Executar query EQS para posicionamento
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void ExecuteTacticalPositioning(UEnvQuery* Query);

    // Callback para resultado de EQS
    UFUNCTION()
    void OnEQSQueryComplete(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);

    // === SISTEMA DE COMENTÁRIOS ===

    // Comentar situação de combate (para feedback ao jogador)
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void CommentOnSituation(const FString& SituationType);

    // Emitir som de combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void EmitCombatSound(const FString& SoundType);

    // === EVENTOS DE PERCEPÇÃO ===

    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    UFUNCTION()
    void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

    // === ANÁLISE DE SITUAÇÃO ===

    // Avaliar nível de ameaça da situação atual
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    EThreatLevel EvaluateCurrentThreatLevel();

    // Calcular vantagem tática
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float CalculateTacticalAdvantage();

    // Determinar se deve recuar
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    bool ShouldRetreat();

    // === PERSONALIZAÇÃO ===

    // Gerar personalidade única para este dinossauro
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    void GenerateUniquePersonality();

    // Aplicar modificadores de personalidade
    UFUNCTION(BlueprintCallable, Category = "Combat AI")
    float ApplyPersonalityModifier(float BaseValue, const FString& ModifierType);

private:
    // Timers
    FTimerHandle CombatUpdateTimer;
    FTimerHandle LearningUpdateTimer;
    FTimerHandle CoordinationTimer;

    // Estado interno
    bool bInCombat = false;
    float LastCombatTime = 0.0f;
    int32 ConsecutiveDefeats = 0;
    int32 ConsecutiveVictories = 0;

    // Funções internas
    void UpdateCombatState();
    void UpdateLearning();
    void UpdateCoordination();
    void SelectAndRunBehaviorTree();
    void UpdateBlackboardValues();
    void HandleCombatPhaseTransition();
    void CalculateStressLevel();
    void ApplyIntelligenceModifiers();
};

/**
 * Combat AI Agent - Sistema Principal
 * Gerencia múltiplos Combat AI Controllers e coordena comportamentos
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCombatAIAgent : public UObject
{
    GENERATED_BODY()

public:
    UCombatAIAgent();

    // === GERENCIAMENTO DE CONTROLLERS ===

    // Registrar novo Combat AI Controller
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    void RegisterCombatAI(ACombatAIController* Controller);

    // Remover Combat AI Controller
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    void UnregisterCombatAI(ACombatAIController* Controller);

    // Obter todos os Combat AIs ativos
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    TArray<ACombatAIController*> GetActiveCombatAIs();

    // === COORDENAÇÃO GLOBAL ===

    // Coordenar resposta de grupo a ameaça
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    void CoordinateGroupResponse(AActor* Threat, const FVector& ThreatLocation);

    // Balancear dificuldade dinamicamente
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    void BalanceDifficulty(float PlayerPerformance);

    // === ANÁLISE E MÉTRICAS ===

    // Analisar performance do jogador
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    float AnalyzePlayerPerformance();

    // Obter estatísticas de combate
    UFUNCTION(BlueprintCallable, Category = "Combat AI Agent")
    FString GetCombatStatistics();

    // === CONFIGURAÇÃO ===

    // Configurar dificuldade global
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float GlobalDifficultyMultiplier = 1.0f;

    // Ativar aprendizagem adaptativa
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableAdaptiveLearning = true;

    // Ativar coordenação de grupo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableGroupCoordination = true;

    // Ativar comentários de combate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bEnableCombatCommentary = true;

private:
    // Lista de Combat AI Controllers registrados
    UPROPERTY()
    TArray<ACombatAIController*> RegisteredControllers;

    // Métricas de combate
    UPROPERTY()
    TMap<FString, float> CombatMetrics;

    // Sistema de balanceamento
    float AverageCombatDuration = 30.0f;
    float PlayerWinRate = 0.5f;
    float AveragePlayerHealth = 0.7f;

    // Funções internas
    void UpdateGlobalMetrics();
    void AdjustDifficultyBasedOnMetrics();
    void DistributeThreatInformation(AActor* Threat, const FVector& Location);
};