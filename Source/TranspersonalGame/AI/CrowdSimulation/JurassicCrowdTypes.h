#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "MassEntityTypes.h"
#include "MassCommonTypes.h"
#include "JurassicCrowdTypes.generated.h"

/**
 * Tipos de agentes no ecossistema Jurássico
 */
UENUM(BlueprintType)
enum class EJurassicAgentType : uint8
{
    None = 0,
    
    // Herbívoros - Grandes Manadas
    Triceratops,
    Brachiosaurus,
    Parasaurolophus,
    Stegosaurus,
    
    // Herbívoros - Pequenos Grupos
    Gallimimus,
    Compsognathus,
    
    // Predadores - Solitários
    TyrannosaurusRex,
    Allosaurus,
    Carnotaurus,
    
    // Predadores - Matilha
    Velociraptor,
    Deinonychus,
    
    // Vida Selvagem Ambiente
    Pteranodon,
    Dimorphodon,
    SmallMammals,
    
    MAX UMETA(Hidden)
};

/**
 * Estados comportamentais dos agentes
 */
UENUM(BlueprintType)
enum class EJurassicBehaviorState : uint8
{
    Idle = 0,
    Grazing,        // Herbívoros pastando
    Drinking,       // Bebendo água
    Migrating,      // Migração sazonal
    Hunting,        // Predadores caçando
    Fleeing,        // Fugindo de ameaça
    Socializing,    // Interação social
    Resting,        // Descansando
    Territorial,    // Defendendo território
    Mating,         // Comportamento reprodutivo
    Investigating,  // Investigando distúrbio
    
    MAX UMETA(Hidden)
};

/**
 * Níveis de alerta no ecossistema
 */
UENUM(BlueprintType)
enum class EJurassicAlertLevel : uint8
{
    Calm = 0,       // Estado normal
    Cautious,       // Ligeiramente alerta
    Nervous,        // Nervoso, pronto para fugir
    Panicked,       // Pânico total
    Aggressive,     // Modo agressivo (predadores)
    
    MAX UMETA(Hidden)
};

/**
 * Configuração de comportamento por espécie
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicSpeciesConfig : public FTableRowBase
{
    GENERATED_BODY()

    FJurassicSpeciesConfig()
    {
        AgentType = EJurassicAgentType::None;
        PreferredGroupSize = FInt32Range(1, 1);
        MovementSpeed = 300.0f;
        FleeSpeed = 600.0f;
        DetectionRadius = 1500.0f;
        FleeDistance = 2000.0f;
        TerritoryRadius = 3000.0f;
        SocialDistance = 200.0f;
        bIsHerbivore = true;
        bIsPredator = false;
        bCanFly = false;
        DietPreference = 1.0f;
        AggressionLevel = 0.0f;
        HerdCohesion = 0.8f;
        PanicThreshold = 0.7f;
    }

    // Tipo de agente
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EJurassicAgentType AgentType;

    // Configuração de grupo
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FInt32Range PreferredGroupSize;

    // Velocidades
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FleeSpeed;

    // Distâncias de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float DetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float SocialDistance;

    // Características da espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsPredator;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCanFly;

    // Comportamento
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DietPreference; // 0 = herbívoro puro, 1 = carnívoro puro

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HerdCohesion; // Tendência para ficar em grupo

    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PanicThreshold; // Limiar para entrar em pânico
};

/**
 * Fragment para identificação de espécie
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicSpeciesFragment : public FMassFragment
{
    GENERATED_BODY()

    EJurassicAgentType SpeciesType = EJurassicAgentType::None;
    int32 SpeciesConfigIndex = -1;
};

/**
 * Fragment para estado comportamental
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicBehaviorFragment : public FMassFragment
{
    GENERATED_BODY()

    EJurassicBehaviorState CurrentState = EJurassicBehaviorState::Idle;
    EJurassicBehaviorState PreviousState = EJurassicBehaviorState::Idle;
    EJurassicAlertLevel AlertLevel = EJurassicAlertLevel::Calm;
    
    float StateTimer = 0.0f;
    float StateDuration = 5.0f;
    
    FVector LastKnownThreatLocation = FVector::ZeroVector;
    float ThreatLevel = 0.0f;
    float LastThreatTime = 0.0f;
};

/**
 * Fragment para comportamento de grupo/manada
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicHerdFragment : public FMassFragment
{
    GENERATED_BODY()

    int32 HerdID = -1;
    bool bIsHerdLeader = false;
    FVector HerdCenter = FVector::ZeroVector;
    FVector HerdDirection = FVector::ForwardVector;
    
    int32 HerdSize = 1;
    float CohesionStrength = 0.8f;
    float SeparationDistance = 200.0f;
    
    // Para migração
    FVector MigrationTarget = FVector::ZeroVector;
    bool bIsMigrating = false;
};

/**
 * Fragment para território (predadores)
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicTerritoryFragment : public FMassFragment
{
    GENERATED_BODY()

    FVector TerritoryCenter = FVector::ZeroVector;
    float TerritoryRadius = 3000.0f;
    
    TArray<FVector> PatrolPoints;
    int32 CurrentPatrolIndex = 0;
    
    float LastHuntTime = 0.0f;
    float HuntCooldown = 300.0f; // 5 minutos entre caçadas
};

/**
 * Fragment para necessidades básicas
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicNeedsFragment : public FMassFragment
{
    GENERATED_BODY()

    float Hunger = 0.5f;        // 0 = saciado, 1 = faminto
    float Thirst = 0.5f;        // 0 = hidratado, 1 = sedento
    float Energy = 1.0f;        // 0 = exausto, 1 = energético
    float Social = 0.5f;        // 0 = solitário, 1 = precisa de companhia
    
    float HungerRate = 0.1f;    // Taxa de aumento da fome por minuto
    float ThirstRate = 0.15f;   // Taxa de aumento da sede por minuto
    float EnergyRate = 0.05f;   // Taxa de diminuição da energia por minuto
};

/**
 * Tag para diferentes tipos de agentes
 */
USTRUCT()
struct TRANSPERSONALGAME_API FJurassicHerbivoreTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FJurassicPredatorTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FJurassicFlyingTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FJurassicHerdLeaderTag : public FMassTag
{
    GENERATED_BODY()
};

USTRUCT()
struct TRANSPERSONALGAME_API FJurassicPanickedTag : public FMassTag
{
    GENERATED_BODY()
};