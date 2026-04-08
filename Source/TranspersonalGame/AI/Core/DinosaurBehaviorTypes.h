#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DinosaurBehaviorTypes.generated.h"

/**
 * Enumeração dos tipos comportamentais base dos dinossauros
 * Cada tipo define padrões fundamentais de comportamento
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorType : uint8
{
    // Herbívoros passivos - potencialmente domesticáveis
    PassiveHerbivore     UMETA(DisplayName = "Herbívoro Passivo"),
    
    // Herbívoros defensivos - fogem mas podem atacar se encurralados
    DefensiveHerbivore   UMETA(DisplayName = "Herbívoro Defensivo"),
    
    // Carnívoros pequenos - caçam em grupos, evitam grandes predadores
    SmallCarnivore       UMETA(DisplayName = "Carnívoro Pequeno"),
    
    // Carnívoros médios - caçadores oportunistas
    MediumCarnivore      UMETA(DisplayName = "Carnívoro Médio"),
    
    // Grandes predadores - apex predators, territorialistas
    ApexPredator         UMETA(DisplayName = "Predador Apex"),
    
    // Carniceiros - seguem predadores, limpam restos
    Scavenger           UMETA(DisplayName = "Carniceiro"),
    
    // Aquáticos - comportamento específico para água
    Aquatic             UMETA(DisplayName = "Aquático"),
    
    // Voadores - patrulham do ar, diferentes padrões de movimento
    Flying              UMETA(DisplayName = "Voador")
};

/**
 * Estados emocionais que afetam o comportamento
 */
UENUM(BlueprintType)
enum class EDinosaurEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calmo"),
    Alert       UMETA(DisplayName = "Alerta"),
    Aggressive  UMETA(DisplayName = "Agressivo"),
    Fearful     UMETA(DisplayName = "Amedrontado"),
    Hungry      UMETA(DisplayName = "Faminto"),
    Territorial UMETA(DisplayName = "Territorial"),
    Protective  UMETA(DisplayName = "Protetor"),
    Curious     UMETA(DisplayName = "Curioso"),
    Tired       UMETA(DisplayName = "Cansado"),
    Injured     UMETA(DisplayName = "Ferido")
};

/**
 * Atividades que os dinossauros podem estar executando
 */
UENUM(BlueprintType)
enum class EDinosaurActivity : uint8
{
    // Atividades básicas de sobrevivência
    Foraging    UMETA(DisplayName = "Procurando Comida"),
    Drinking    UMETA(DisplayName = "Bebendo Água"),
    Resting     UMETA(DisplayName = "Descansando"),
    Sleeping    UMETA(DisplayName = "Dormindo"),
    
    // Atividades sociais
    Socializing UMETA(DisplayName = "Socializando"),
    Mating      UMETA(DisplayName = "Acasalando"),
    Protecting  UMETA(DisplayName = "Protegendo"),
    
    // Atividades de movimento
    Patrolling  UMETA(DisplayName = "Patrulhando"),
    Migrating   UMETA(DisplayName = "Migrando"),
    Fleeing     UMETA(DisplayName = "Fugindo"),
    Hunting     UMETA(DisplayName = "Caçando"),
    
    // Atividades de manutenção
    Grooming    UMETA(DisplayName = "Limpando-se"),
    Sunbathing  UMETA(DisplayName = "Tomando Sol"),
    Marking     UMETA(DisplayName = "Marcando Território"),
    
    // Interação com jogador
    Investigating UMETA(DisplayName = "Investigando"),
    Bonding     UMETA(DisplayName = "Criando Vínculo"),
    Following   UMETA(DisplayName = "Seguindo")
};

/**
 * Níveis de domesticação para herbívoros pequenos
 */
UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild            UMETA(DisplayName = "Selvagem"),
    Wary            UMETA(DisplayName = "Desconfiado"),
    Curious         UMETA(DisplayName = "Curioso"),
    Tolerant        UMETA(DisplayName = "Tolerante"),
    Friendly        UMETA(DisplayName = "Amigável"),
    Bonded          UMETA(DisplayName = "Vinculado"),
    Domesticated    UMETA(DisplayName = "Domesticado")
};

/**
 * Estrutura para definir rotinas diárias
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutine
{
    GENERATED_BODY()

    // Hora do dia (0.0 = meia-noite, 0.5 = meio-dia, 1.0 = meia-noite)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TimeOfDay = 0.0f;

    // Atividade principal neste período
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurActivity PrimaryActivity = EDinosaurActivity::Resting;

    // Localização preferida para esta atividade (relativa ao território)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    // Duração da atividade em horas de jogo
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "12.0"))
    float Duration = 1.0f;

    // Prioridade desta rotina (maior = mais importante)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", ClampMax = "10"))
    int32 Priority = 5;
};

/**
 * Configuração comportamental específica por espécie
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UDinosaurBehaviorConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    // Tipo comportamental base
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    EDinosaurBehaviorType BehaviorType = EDinosaurBehaviorType::PassiveHerbivore;

    // Nome da espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    FString SpeciesName = TEXT("Unknown Species");

    // Pode ser domesticado?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domestication")
    bool bCanBeDomesticated = false;

    // Tempo necessário para domesticação completa (em horas de jogo)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domestication", 
              meta = (EditCondition = "bCanBeDomesticated", ClampMin = "1.0", ClampMax = "100.0"))
    float DomesticationTimeRequired = 24.0f;

    // Rotinas diárias desta espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Daily Routine")
    TArray<FDailyRoutine> DailyRoutines;

    // Raio do território em metros
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Territory", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
    float TerritoryRadius = 500.0f;

    // Distância de detecção do jogador
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception", meta = (ClampMin = "100.0", ClampMax = "2000.0"))
    float PlayerDetectionRange = 300.0f;

    // Distância de fuga (quando começa a fugir do jogador)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Perception", meta = (ClampMin = "50.0", ClampMax = "1000.0"))
    float FleeDistance = 150.0f;

    // Velocidade de movimento normal (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "50.0", ClampMax = "2000.0"))
    float NormalSpeed = 200.0f;

    // Velocidade de fuga (cm/s)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement", meta = (ClampMin = "200.0", ClampMax = "4000.0"))
    float FleeSpeed = 600.0f;

    // Tags de gameplay para este comportamento
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FGameplayTagContainer BehaviorTags;

    // Sons específicos desta espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    TMap<FString, class USoundBase*> SpeciesSounds;
};