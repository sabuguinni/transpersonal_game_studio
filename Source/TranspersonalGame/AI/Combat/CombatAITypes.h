#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "CombatAITypes.generated.h"

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Passive,        // Herbívoros pacíficos
    Defensive,      // Herbívoros que defendem território
    Opportunist,    // Carnívoros pequenos em grupos
    Apex,          // Predadores solitários grandes
    Alpha          // Líderes de matilha
};

UENUM(BlueprintType)
enum class ECombatBehaviorState : uint8
{
    Patrol,         // Patrulhamento normal
    Investigate,    // Investigando distúrbio
    Hunt,          // Caçando presa identificada
    Stalk,         // Seguindo discretamente
    Ambush,        // Preparando emboscada
    Attack,        // Ataque direto
    Retreat,       // Recuando ferido
    Feeding,       // Alimentando-se
    Territorial    // Defendendo território
};

UENUM(BlueprintType)
enum class ECombatFormation : uint8
{
    Solo,          // Caçador solitário
    Pack,          // Matilha coordenada
    Ambush,        // Emboscada múltipla
    Pincer,        // Movimento pinça
    Distraction    // Um distrai, outros atacam
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurCombatProfile : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DetectionRange = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanFormPacks = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PreferredPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritoryRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer CombatTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<ECombatFormation> AvailableFormations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbushProbability = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RetreatHealthThreshold = 0.25f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCombatContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    AActor* Target = nullptr;

    UPROPERTY(BlueprintReadWrite)
    FVector LastKnownTargetLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite)
    float TimeSinceLastSeen = 0.0f;

    UPROPERTY(BlueprintReadWrite)
    ECombatBehaviorState CurrentState = ECombatBehaviorState::Patrol;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> PackMembers;

    UPROPERTY(BlueprintReadWrite)
    AActor* PackLeader = nullptr;

    UPROPERTY(BlueprintReadWrite)
    ECombatFormation ActiveFormation = ECombatFormation::Solo;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAmbushReady = false;

    UPROPERTY(BlueprintReadWrite)
    float CurrentThreatLevel = 0.0f;
};