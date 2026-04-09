#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "NPCBehaviorCore.h"
#include "NPCMemorySystem.generated.h"

/**
 * SISTEMA DE MEMÓRIA NPC - AGENTE #11
 * 
 * "A memória define quem somos. Um NPC sem memória é apenas um autómato."
 * 
 * Sistema avançado de memória que permite aos NPCs:
 * - Lembrar eventos específicos com degradação temporal realista
 * - Formar associações emocionais com locais e pessoas
 * - Aprender com experiências passadas
 * - Desenvolver preferências baseadas em memórias
 * - Reagir diferentemente baseado no histórico
 * 
 * INSPIRAÇÃO: Westworld + Ex Machina + Her
 */

UENUM(BlueprintType)
enum class EMemoryType : uint8
{
    Personal               UMETA(DisplayName = "Personal Experience"),
    Social                 UMETA(DisplayName = "Social Interaction"),
    Location               UMETA(DisplayName = "Location Memory"),
    Object                 UMETA(DisplayName = "Object Memory"),
    Skill                  UMETA(DisplayName = "Skill Learning"),
    Emotional              UMETA(DisplayName = "Emotional Memory"),
    Procedural             UMETA(DisplayName = "Procedural Memory"),
    Episodic               UMETA(DisplayName = "Episodic Memory"),
    Semantic               UMETA(DisplayName = "Semantic Memory"),
    
    MAX                    UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EMemoryImportance : uint8
{
    Trivial                UMETA(DisplayName = "Trivial"),
    Minor                  UMETA(DisplayName = "Minor"),
    Moderate               UMETA(DisplayName = "Moderate"),
    Important              UMETA(DisplayName = "Important"),
    Critical               UMETA(DisplayName = "Critical"),
    LifeChanging           UMETA(DisplayName = "Life Changing"),
    
    MAX                    UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryAssociation
{
    GENERATED_BODY()

    FMemoryAssociation()
        : AssociatedObjectID(TEXT(""))
        , AssociationType(TEXT(""))
        , EmotionalValue(0.0f)
        , Strength(1.0f)
        , LastReinforced(FDateTime::Now())
    {}

    // ID do objeto/pessoa/local associado
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssociatedObjectID;
    
    // Tipo de associação (friend, enemy, dangerous_place, safe_place, etc.)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AssociationType;
    
    // Valor emocional da associação (-1.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalValue;
    
    // Força da associação (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Strength;
    
    // Última vez que foi reforçada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastReinforced;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDetailedMemoryEvent
{
    GENERATED_BODY()

    FDetailedMemoryEvent()
        : EventID(TEXT(""))
        , MemoryType(EMemoryType::Personal)
        , EventType(TEXT(""))
        , Description(TEXT(""))
        , Location(FVector::ZeroVector)
        , Timestamp(FDateTime::Now())
        , EmotionalImpact(0.0f)
        , Importance(EMemoryImportance::Moderate)
        , bIsPositive(true)
        , DecayRate(0.01f)
        , CurrentStrength(1.0f)
        , AccessCount(0)
        , LastAccessed(FDateTime::Now())
    {}

    // Identificação única do evento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventID;
    
    // Tipo de memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryType MemoryType;
    
    // Tipo específico do evento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventType;
    
    // Descrição detalhada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;
    
    // Local onde ocorreu
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;
    
    // Quando ocorreu
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
    
    // Impacto emocional (-1.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalImpact;
    
    // Importância da memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryImportance Importance;
    
    // Se é uma memória positiva ou negativa
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPositive;
    
    // Taxa de degradação da memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate;
    
    // Força atual da memória (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CurrentStrength;
    
    // Quantas vezes foi acessada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AccessCount;
    
    // Última vez que foi acessada
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastAccessed;
    
    // Associações desta memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FMemoryAssociation> Associations;
    
    // Participantes do evento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Participants;
    
    // Tags para busca rápida
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;
    
    // Dados contextuais específicos
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> ContextualData;
};

/**
 * Componente de Sistema de Memória NPC
 * Gerencia todas as memórias e associações de um NPC
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCMemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCMemorySystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === GESTÃO DE MEMÓRIAS ===
    
    /**
     * Adiciona uma nova memória ao sistema
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FDetailedMemoryEvent& NewMemory);
    
    /**
     * Recupera memórias por tipo
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetMemoriesByType(EMemoryType MemoryType) const;
    
    /**
     * Recupera memórias por tag
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetMemoriesByTag(const FString& Tag) const;
    
    /**
     * Recupera memórias relacionadas a um local específico
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetMemoriesNearLocation(const FVector& Location, float Radius) const;
    
    /**
     * Recupera memórias envolvendo uma pessoa específica
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetMemoriesWithParticipant(const FString& ParticipantID) const;
    
    /**
     * Recupera as memórias mais importantes
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetMostImportantMemories(int32 Count = 10) const;
    
    /**
     * Recupera memórias recentes
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetRecentMemories(float HoursBack = 24.0f) const;
    
    // === SISTEMA DE ASSOCIAÇÕES ===
    
    /**
     * Adiciona uma associação emocional
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddAssociation(const FString& ObjectID, const FString& AssociationType, float EmotionalValue);
    
    /**
     * Recupera associação com um objeto/pessoa
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FMemoryAssociation GetAssociation(const FString& ObjectID) const;
    
    /**
     * Verifica se tem associação positiva com algo
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasPositiveAssociation(const FString& ObjectID) const;
    
    /**
     * Verifica se tem associação negativa com algo
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasNegativeAssociation(const FString& ObjectID) const;
    
    /**
     * Reforça uma associação existente
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ReinforceAssociation(const FString& ObjectID, float Strength);
    
    // === DEGRADAÇÃO E MANUTENÇÃO ===
    
    /**
     * Aplica degradação temporal às memórias
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ApplyMemoryDecay(float DeltaTime);
    
    /**
     * Remove memórias muito fracas
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void CleanupWeakMemories();
    
    /**
     * Consolida memórias similares
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ConsolidateMemories();
    
    // === ANÁLISE E DECISÃO ===
    
    /**
     * Avalia o valor emocional de um local baseado em memórias
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float EvaluateLocationEmotionalValue(const FVector& Location, float Radius = 1000.0f) const;
    
    /**
     * Avalia a confiança em uma pessoa baseada em memórias
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float EvaluatePersonTrust(const FString& PersonID) const;
    
    /**
     * Determina se deve evitar um local baseado em memórias
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool ShouldAvoidLocation(const FVector& Location, float Radius = 500.0f) const;
    
    /**
     * Recupera memórias relevantes para uma decisão
     */
    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FDetailedMemoryEvent> GetRelevantMemoriesForDecision(const FString& DecisionContext) const;

protected:
    // === CONFIGURAÇÃO ===
    
    // Capacidade máxima de memórias
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxMemoryCapacity;
    
    // Taxa base de degradação
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float BaseDecayRate;
    
    // Força mínima para manter uma memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MinMemoryStrength;
    
    // Frequência de limpeza automática (segundos)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float CleanupFrequency;
    
    // === DADOS ===
    
    // Todas as memórias do NPC
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    TArray<FDetailedMemoryEvent> Memories;
    
    // Associações emocionais
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Data")
    TMap<FString, FMemoryAssociation> Associations;
    
    // Timer para limpeza automática
    float CleanupTimer;
    
    // === MÉTODOS INTERNOS ===
    
    /**
     * Calcula a força de uma memória baseada em vários fatores
     */
    float CalculateMemoryStrength(const FDetailedMemoryEvent& Memory) const;
    
    /**
     * Encontra memórias similares para consolidação
     */
    TArray<int32> FindSimilarMemories(const FDetailedMemoryEvent& Memory) const;
    
    /**
     * Calcula a similaridade entre duas memórias
     */
    float CalculateMemorySimilarity(const FDetailedMemoryEvent& Memory1, const FDetailedMemoryEvent& Memory2) const;
    
    /**
     * Gera ID único para uma memória
     */
    FString GenerateMemoryID() const;
    
    /**
     * Aplica modificadores de personalidade à força da memória
     */
    float ApplyPersonalityModifiers(float BaseStrength, const FDetailedMemoryEvent& Memory) const;

public:
    // === EVENTOS ===
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryAdded, const FDetailedMemoryEvent&, Memory);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryForgotten, const FString&, MemoryID);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAssociationChanged, const FString&, ObjectID, float, NewEmotionalValue);
    
    UPROPERTY(BlueprintAssignable)
    FOnMemoryAdded OnMemoryAdded;
    
    UPROPERTY(BlueprintAssignable)
    FOnMemoryForgotten OnMemoryForgotten;
    
    UPROPERTY(BlueprintAssignable)
    FOnAssociationChanged OnAssociationChanged;
};