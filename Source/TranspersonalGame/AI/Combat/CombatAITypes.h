#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameplayTags.h"
#include "CombatAITypes.generated.h"

/**
 * Tipos de comportamento de combate para dinossauros
 */
UENUM(BlueprintType)
enum class EDinosaurCombatType : uint8
{
    Predator_Apex      UMETA(DisplayName = "Apex Predator"),     // T-Rex, Giganotosaurus
    Predator_Pack      UMETA(DisplayName = "Pack Hunter"),      // Velociraptor, Deinonychus
    Predator_Ambush    UMETA(DisplayName = "Ambush Predator"),  // Carnotaurus, Baryonyx
    Herbivore_Defensive UMETA(DisplayName = "Defensive Herbivore"), // Triceratops, Ankylosaurus
    Herbivore_Fleeing  UMETA(DisplayName = "Fleeing Herbivore"), // Parasaurolophus, Gallimimus
    Scavenger         UMETA(DisplayName = "Scavenger")          // Compsognathus (grupos)
};

/**
 * Estados de alerta do dinossauro
 */
UENUM(BlueprintType)
enum class EDinosaurAlertState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Curious     UMETA(DisplayName = "Curious"),
    Suspicious  UMETA(DisplayName = "Suspicious"),
    Alert       UMETA(DisplayName = "Alert"),
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Hunting     UMETA(DisplayName = "Hunting"),
    Fleeing     UMETA(DisplayName = "Fleeing")
};

/**
 * Tipos de ataque disponíveis
 */
UENUM(BlueprintType)
enum class EDinosaurAttackType : uint8
{
    Bite        UMETA(DisplayName = "Bite"),
    Claw        UMETA(DisplayName = "Claw"),
    Tail        UMETA(DisplayName = "Tail Whip"),
    Charge      UMETA(DisplayName = "Charge"),
    Stomp       UMETA(DisplayName = "Stomp"),
    Pounce      UMETA(DisplayName = "Pounce")
};

/**
 * Estrutura de dados para ataques
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurAttack
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Damage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Range = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Cooldown = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WindupTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RecoveryTime = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTag AttackTag;
};

/**
 * Configuração de comportamento de combate
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatBehaviorConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurCombatType CombatType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float OptimalCombatRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FDinosaurAttack> AvailableAttacks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanCallForHelp = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HelpCallRadius = 2000.0f;
};