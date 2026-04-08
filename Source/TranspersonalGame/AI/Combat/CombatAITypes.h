#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTags.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    None = 0,
    Passive,        // Herbívoros pacíficos
    Defensive,      // Reagem quando ameaçados
    Territorial,    // Defendem área específica
    Aggressive,     // Caçam ativamente
    Apex           // Predadores supremos
};

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    Idle = 0,
    Patrolling,
    Investigating,
    Stalking,
    Engaging,
    Retreating,
    Feeding,
    Resting
};

UENUM(BlueprintType)
enum class EAttackPattern : uint8
{
    None = 0,
    Ambush,         // Ataque surpresa
    Charge,         // Investida direta
    Circle,         // Cercar a presa
    Pack,           // Coordenação de grupo
    Territorial     // Defesa de área
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatPersonality
{
    GENERATED_BODY()

    // Agressividade base (0.0 = pacífico, 1.0 = extremamente agressivo)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;

    // Curiosidade (tendência a investigar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.3f;

    // Cautela (tendência a recuar)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Caution = 0.4f;

    // Territorialidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;

    // Tendência a formar grupos
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.2f;

    // Inteligência tática
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatCapabilities
{
    GENERATED_BODY()

    // Dano base de ataque
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseDamage = 10.0f;

    // Alcance de ataque
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 200.0f;

    // Velocidade de movimento
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 400.0f;

    // Velocidade de rotação
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TurnRate = 90.0f;

    // Resistência a dano
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageResistance = 1.0f;

    // Alcance de detecção
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1000.0f;

    // Campo de visão (em graus)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FieldOfView = 120.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatMemory
{
    GENERATED_BODY()

    // Última posição conhecida do jogador
    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;

    // Tempo desde a última detecção
    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastDetection = 0.0f;

    // Nível de alerta atual
    UPROPERTY(BlueprintReadWrite)
    float AlertLevel = 0.0f;

    // Locais de interesse investigados
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> InvestigatedLocations;

    // Dano recebido recentemente
    UPROPERTY(BlueprintReadWrite)
    float RecentDamage = 0.0f;

    // Tempo desde o último ataque
    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastAttack = 0.0f;
};

/**
 * Data Asset que define as características de combate de uma espécie de dinossauro
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCombatSpeciesData : public UDataAsset
{
    GENERATED_BODY()

public:
    // Nome da espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Species")
    FString SpeciesName;

    // Nível de ameaça base
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    EThreatLevel ThreatLevel = EThreatLevel::Passive;

    // Padrões de ataque preferidos
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    TArray<EAttackPattern> PreferredAttackPatterns;

    // Personalidade base da espécie
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality")
    FCombatPersonality BasePersonality;

    // Capacidades de combate
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Capabilities")
    FCombatCapabilities CombatCapabilities;

    // Tags de gameplay associadas
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay")
    FGameplayTagContainer SpeciesTags;

    // Pode ser domesticado?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    bool bCanBeTamed = false;

    // Tempo necessário para domesticação (em segundos)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (EditCondition = "bCanBeTamed"))
    float TamingTime = 300.0f;

    // Requer comida específica para domesticação?
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior", meta = (EditCondition = "bCanBeTamed"))
    bool bRequiresSpecificFood = false;
};