#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

// Enum para tipos de dinossauros baseado em comportamento
UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    ApexPredator,      // T-Rex, Giganotosaurus - dominam território
    PackHunter,        // Velociraptor, Deinonychus - caçam em grupo
    SoloPredator,      // Carnotaurus, Allosaurus - caçam sozinhos
    HerdHerbivore,     // Triceratops, Parasaurolophus - vivem em grupos
    SoloHerbivore,     // Ankylosaurus, Stegosaurus - territoriais
    SmallHerbivore,    // Compsognathus, Dryosaurus - domesticáveis
    Scavenger,         // Compsognathus (modo alternativo)
    Aquatic,           // Mosasaurus, Plesiosaur
    Flying             // Pteranodon, Quetzalcoatlus
};

// Estados comportamentais base
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Hunting,
    Fleeing,
    Socializing,
    Resting,
    Drinking,
    Territorial,
    Mating,
    Nesting,
    Migrating,
    Investigating,
    Domestication    // Estado especial para processo de domesticação
};

// Necessidades básicas que influenciam comportamento
UENUM(BlueprintType)
enum class EDinosaurNeed : uint8
{
    Hunger,
    Thirst,
    Safety,
    Social,
    Territory,
    Rest,
    Reproduction
};

// Estrutura para definir rotinas diárias
USTRUCT(BlueprintType)
struct FDinosaurDailyRoutine : public FTableRowBase
{
    GENERATED_BODY()

    // Hora do dia (0-24) quando esta atividade é mais provável
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredTimeStart = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PreferredTimeEnd = 18.0f;

    // Estado comportamental durante este período
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState BehaviorState = EDinosaurBehaviorState::Idle;

    // Prioridade desta atividade (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 0.5f;

    // Duração mínima e máxima da atividade
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinDuration = 300.0f; // 5 minutos

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaxDuration = 1800.0f; // 30 minutos

    // Locais preferidos para esta atividade
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FGameplayTag> PreferredLocationTags;
};

// Perfil comportamental completo de uma espécie
USTRUCT(BlueprintType)
struct FDinosaurSpeciesProfile : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurArchetype Archetype = EDinosaurArchetype::SoloHerbivore;

    // Rotinas diárias desta espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDinosaurDailyRoutine> DailyRoutines;

    // Agressividade base (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseAggression = 0.3f;

    // Curiosidade base (0-1)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseCuriosity = 0.5f;

    // Tendência social (0-1, onde 1 = muito social)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialTendency = 0.5f;

    // Pode ser domesticado?
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeDomesticated = false;

    // Tempo necessário para domesticação completa (em segundos de jogo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bCanBeDomesticated"))
    float DomesticationTime = 7200.0f; // 2 horas de jogo

    // Distância de detecção do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerDetectionRange = 1500.0f;

    // Distância de fuga do jogador (se aplicável)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FleeDistance = 800.0f;

    // Espécies que são presas naturais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PreySpecies;

    // Espécies que são predadores naturais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PredatorSpecies;

    // Tags de comportamento especiais
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer BehaviorTags;
};

// Sistema de memória individual para cada dinossauro
USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Locais importantes conhecidos
    UPROPERTY(BlueprintReadWrite)
    TMap<FGameplayTag, FVector> KnownLocations;

    // Últimas interações com o jogador
    UPROPERTY(BlueprintReadWrite)
    TArray<float> PlayerInteractionHistory; // Timestamps das interações

    // Nível de confiança com o jogador (para domesticação)
    UPROPERTY(BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f;

    // Outros dinossauros conhecidos
    UPROPERTY(BlueprintReadWrite)
    TMap<int32, float> KnownDinosaurs; // ID -> Relação (-1 a 1)

    // Locais perigosos conhecidos
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> DangerousLocations;

    // Tempo da última atualização de memória
    UPROPERTY(BlueprintReadWrite)
    float LastMemoryUpdate = 0.0f;
};