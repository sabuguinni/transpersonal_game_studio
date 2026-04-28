#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
// DISABLED: #include "Perception/PerceptionComponent.h"
// FIXME: Missing header - #include "Characters/CharacterArchetypes.h"
#include "NPCBehaviorCore.generated.h"

class UBehaviorTreeComponent;
class UBlackboardComponent;
class AAIController;

/**
 * NPC BEHAVIOR CORE SYSTEM - AGENTE #11
 * 
 * "Cada NPC vive a sua vida. O jogador é apenas uma interrupção nessa vida."
 * 
 * Sistema avançado de comportamento NPC inspirado pela filosofia de que personagens
 * emergentes são mais memoráveis que personagens roteirizados. Cada NPC tem:
 * 
 * - Personalidade única que afeta todas as decisões
 * - Memória persistente de eventos e relações
 * - Rotinas diárias que variam baseadas em personalidade
 * - Sistema de relações sociais dinâmico
 * - Reações emergentes baseadas em contexto
 * - Objetivos pessoais que evoluem com o tempo
 * 
 * INSPIRAÇÃO: Rockstar Games (RDR2) + Tynan Sylvester (RimWorld)
 */

UENUM(BlueprintType)
enum class ENPCPersonalityTrait : uint8
{
    // Traços de personalidade base
    Introvert               UMETA(DisplayName = "Introvert"),
    Extrovert              UMETA(DisplayName = "Extrovert"),
    Cautious               UMETA(DisplayName = "Cautious"),
    Brave                  UMETA(DisplayName = "Brave"),
    Optimistic             UMETA(DisplayName = "Optimistic"),
    Pessimistic            UMETA(DisplayName = "Pessimistic"),
    Analytical             UMETA(DisplayName = "Analytical"),
    Intuitive              UMETA(DisplayName = "Intuitive"),
    Cooperative            UMETA(DisplayName = "Cooperative"),
    Competitive            UMETA(DisplayName = "Competitive"),
    
    // Traços específicos do contexto pré-histórico
    NaturalSurvivor        UMETA(DisplayName = "Natural Survivor"),
    ScientificMind         UMETA(DisplayName = "Scientific Mind"),
    AdaptabilityHigh       UMETA(DisplayName = "High Adaptability"),
    TraditionBound         UMETA(DisplayName = "Tradition Bound"),
    ResourceHoarder        UMETA(DisplayName = "Resource Hoarder"),
    ResourceSharer         UMETA(DisplayName = "Resource Sharer"),
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EAI_NPCEmotionalState_6D3 : uint8
{
    Neutral                UMETA(DisplayName = "Neutral"),
    Happy                  UMETA(DisplayName = "Happy"),
    Sad                    UMETA(DisplayName = "Sad"),
    Angry                  UMETA(DisplayName = "Angry"),
    Fearful                UMETA(DisplayName = "Fearful"),
    Excited                UMETA(DisplayName = "Excited"),
    Anxious                UMETA(DisplayName = "Anxious"),
    Curious                UMETA(DisplayName = "Curious"),
    Determined             UMETA(DisplayName = "Determined"),
    Hopeless               UMETA(DisplayName = "Hopeless"),
    Grateful               UMETA(DisplayName = "Grateful"),
    Suspicious             UMETA(DisplayName = "Suspicious"),
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EAI_NPCActivity : uint8
{
    // Atividades básicas
    Idle                   UMETA(DisplayName = "Idle"),
    Sleeping               UMETA(DisplayName = "Sleeping"),
    Eating                 UMETA(DisplayName = "Eating"),
    Drinking               UMETA(DisplayName = "Drinking"),
    Resting                UMETA(DisplayName = "Resting"),
    
    // Atividades de sobrevivência
    GatheringFood          UMETA(DisplayName = "Gathering Food"),
    GatheringWater         UMETA(DisplayName = "Gathering Water"),
    GatheringMaterials     UMETA(DisplayName = "Gathering Materials"),
    Crafting               UMETA(DisplayName = "Crafting"),
    Building               UMETA(DisplayName = "Building"),
    Hunting                UMETA(DisplayName = "Hunting"),
    
    // Atividades sociais
    Socializing            UMETA(DisplayName = "Socializing"),
    Teaching               UMETA(DisplayName = "Teaching"),
    Learning               UMETA(DisplayName = "Learning"),
    Trading                UMETA(DisplayName = "Trading"),
    Helping                UMETA(DisplayName = "Helping"),
    
    // Atividades específicas
    Researching            UMETA(DisplayName = "Researching"),
    Exploring              UMETA(DisplayName = "Exploring"),
    Patrolling             UMETA(DisplayName = "Patrolling"),
    Guarding               UMETA(DisplayName = "Guarding"),
    Meditating             UMETA(DisplayName = "Meditating"),
    Storytelling           UMETA(DisplayName = "Storytelling"),
    
    // Estados de emergência
    Fleeing                UMETA(DisplayName = "Fleeing"),
    Hiding                 UMETA(DisplayName = "Hiding"),
    Fighting               UMETA(DisplayName = "Fighting"),
    Rescuing               UMETA(DisplayName = "Rescuing"),
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EAI_NPCRelationshipType_6D3 : uint8
{
    Stranger               UMETA(DisplayName = "Stranger"),
    Acquaintance           UMETA(DisplayName = "Acquaintance"),
    Friend                 UMETA(DisplayName = "Friend"),
    CloseFriend            UMETA(DisplayName = "Close Friend"),
    Rival                  UMETA(DisplayName = "Rival"),
    Enemy                  UMETA(DisplayName = "Enemy"),
    Mentor                 UMETA(DisplayName = "Mentor"),
    Student                UMETA(DisplayName = "Student"),
    Ally                   UMETA(DisplayName = "Ally"),
    Leader                 UMETA(DisplayName = "Leader"),
    Follower               UMETA(DisplayName = "Follower"),
    
    MAX                    UMETA(Hidden)
};

/**
 * Estrutura que define a personalidade única de um NPC
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_NPCPersonality
{
    GENERATED_BODY()

    FAI_NPCPersonality()
        : Openness(0.5f)
        , Conscientiousness(0.5f)
        , Extraversion(0.5f)
        , Agreeableness(0.5f)
        , Neuroticism(0.5f)
        , Curiosity(0.5f)
        , Empathy(0.5f)
        , Ambition(0.5f)
        , Loyalty(0.5f)
        , Independence(0.5f)
        , RiskTolerance(0.5f)
        , SocialNeed(0.5f)
        , TrustLevel(0.5f)
        , AdaptabilityRate(0.5f)
        , StressResistance(0.5f)
    {}

    // Big Five Personality Traits (0.0 - 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Openness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Conscientiousness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Extraversion;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Agreeableness;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Neuroticism;

    // Traços específicos do contexto
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Empathy;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Ambition;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Loyalty;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Independence;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RiskTolerance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialNeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TrustLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AdaptabilityRate;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StressResistance;

    // Traços dominantes (calculados automaticamente)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TArray<ENPCPersonalityTrait> DominantTraits;

    // Gera traços dominantes baseados nos valores
    void CalculateDominantTraits();
    
    // Calcula compatibilidade com outra personalidade (0.0 - 1.0)
    float CalculateCompatibility(const FAI_NPCPersonality& Other) const;
    
    // Modifica personalidade baseada em experiências
    void ModifyFromExperience(const FString& ExperienceType, float Intensity);
};

/**
 * Memória de um evento específico
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCMemoryEvent
{
    GENERATED_BODY()

    FNPCMemoryEvent()
        : EventID(TEXT(""))
        , EventType(TEXT(""))
        , Description(TEXT(""))
        , Location(FVector::ZeroVector)
        , Timestamp(FDateTime::Now())
        , EmotionalImpact(0.0f)
        , Importance(0.5f)
        , bIsPositive(true)
        , DecayRate(0.01f)
        , CurrentStrength(1.0f)
    {}

    // Identificação do evento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    // Contexto espacial e temporal
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
    
    // Atores envolvidos
// [UHT-FIX2]     UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TWeakObjectPtr<AActor>> InvolvedActors;
    
    // Impacto emocional
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float EmotionalImpact;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Importance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPositive;
    
    // Sistema de decaimento da memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.001", ClampMax = "0.1"))
    float DecayRate;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float CurrentStrength;
    
    // Atualiza a força da memória com o tempo
    void UpdateMemoryStrength(float DeltaTime);
    
    // Verifica se a memória ainda é relevante
    bool IsMemoryRelevant() const { return CurrentStrength > 0.1f; }
};

/**
 * Relacionamento com outro NPC ou o jogador
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAI_NPCRelationship_6D3
{
    GENERATED_BODY()

    FAI_NPCRelationship_6D3()
        : TargetActor(nullptr)
        , TargetName(TEXT(""))
        , RelationshipType(EAI_NPCRelationshipType_6D3::Stranger)
        , TrustLevel(0.0f)
        , AffectionLevel(0.0f)
        , RespectLevel(0.0f)
        , FearLevel(0.0f)
        , LastInteraction(FDateTime::Now())
        , TotalInteractions(0)
        , PositiveInteractions(0)
        , NegativeInteractions(0)
        , bIsPlayerCharacter(false)
    {}

    // Alvo do relacionamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString TargetName;
    
    // Tipo de relacionamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_NPCRelationshipType_6D3 RelationshipType;
    
    // Níveis emocionais (-1.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float TrustLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float AffectionLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float RespectLevel;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearLevel;
    
    // Histórico de interações
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastInteraction;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalInteractions;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions;
    
    // Memórias específicas deste relacionamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNPCMemoryEvent> SharedMemories;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerCharacter;
    
    // Calcula o score geral do relacionamento
    float GetOverallRelationshipScore() const;
    
    // Atualiza o relacionamento baseado numa interação
    void UpdateFromInteraction(const FString& InteractionType, float PositiveImpact, float NegativeImpact);
    
    // Determina o tipo de relacionamento baseado nos scores
    void UpdateRelationshipType();
};

/**
 * Item da rotina diária de um NPC
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCRoutineItem
{
    GENERATED_BODY()

    FNPCRoutineItem()
        : Activity(EAI_NPCActivity::Idle)
        , StartTime(8.0f)
        , Duration(1.0f)
        , Priority(0.5f)
        , Location(FVector::ZeroVector)
        , bIsFlexible(true)
        , FlexibilityRange(0.5f)
        , RequiredItems()
        , RequiredNPCs()
        , Conditions()
        , bCanBeInterrupted(true)
        , InterruptionThreshold(0.7f)
    {}

    // Atividade a realizar
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAI_NPCActivity Activity;
    
    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float StartTime; // Hora do dia (0-24)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "12.0"))
    float Duration; // Duração em horas
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority; // Prioridade da atividade
    
    // Localização
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString LocationName;
    
    // Flexibilidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsFlexible;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FlexibilityRange; // Horas de variação permitida
    
    // Requisitos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredItems;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredNPCs;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Conditions;
    
    // Interrupção
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float InterruptionThreshold; // Quão importante deve ser o evento para interromper
    
    // Verifica se a atividade pode ser executada agora
    bool CanExecuteNow(float CurrentTime, const TArray<FString>& AvailableItems, const TArray<FString>& AvailableNPCs) const;
    
    // Calcula o tempo de início com flexibilidade
    float GetFlexibleStartTime() const;
};

/**
 * Objetivo pessoal de um NPC
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCPersonalGoal
{
    GENERATED_BODY()

    FNPCPersonalGoal()
        : GoalID(TEXT(""))
        , GoalType(TEXT(""))
        , Description(TEXT(""))
        , Priority(0.5f)
        , Progress(0.0f)
        , TargetLocation(FVector::ZeroVector)
        , bIsActive(true)
        , bIsCompleted(false)
        , DeadlineHours(72.0f)
        , CreationTime(FDateTime::Now())
        , MotivationLevel(0.7f)
    {}

    // Identificação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GoalID;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString GoalType;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    // Prioridade e progresso
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Progress;
    
    // Alvos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> TargetActor;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> TargetItems;
    
    // Estado
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsActive;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsCompleted;
    
    // Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DeadlineHours;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime CreationTime;
    
    // Motivação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MotivationLevel;
    
    // Sub-objetivos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SubGoals;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<bool> SubGoalsCompleted;
    
    // Atualiza o progresso do objetivo
    void UpdateProgress(float DeltaProgress);
    
    // Verifica se o objetivo expirou
    bool HasExpired() const;
    
    // Calcula a urgência do objetivo (0.0 - 1.0)
    float GetUrgency() const;
};