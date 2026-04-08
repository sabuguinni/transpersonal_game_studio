#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MetaHumanCharacterBP.h"
#include "CharacterSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist,        // Dr. [Nome] - Paleontologista
    TribalElder,       // Ancião da tribo
    TribalWarrior,     // Guerreiro/Caçador
    TribalHealer,      // Curandeiro/Xamã
    TribalCrafter,     // Artesão/Construtor
    TribalScout,       // Explorador/Batedor
    TribalChild,       // Criança da tribo
    WildHuman,         // Humano selvagem isolado
    TraderNomad,       // Nómada comerciante
    RivalSurvivor      // Sobrevivente rival
};

UENUM(BlueprintType)
enum class ECharacterBodyType : uint8
{
    Lean,              // Magro/Ágil
    Athletic,          // Atlético
    Robust,            // Robusto/Forte
    Elderly,           // Idoso
    Child              // Criança
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    African,
    Asian,
    European,
    Indigenous,
    Mixed
};

USTRUCT(BlueprintType)
struct FCharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VariationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FacialStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BodyMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ClothingVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AccessoryVariations;
};

USTRUCT(BlueprintType)
struct FCharacterArchetypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterBodyType PreferredBodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FCharacterVariation> PossibleVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ClothingStyles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> WeaponTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RarityWeight; // Para spawning procedural

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeNamed; // Se pode ter nome próprio
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterSystem : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* ArchetypeDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<UMetaHumanCharacterBP>> BaseMetaHumans;

    UFUNCTION(BlueprintCallable)
    FCharacterArchetypeData GetArchetypeData(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable)
    UMetaHumanCharacterBP* GenerateRandomCharacter(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable)
    TArray<FCharacterVariation> GenerateVariationsForArchetype(ECharacterArchetype Archetype, int32 Count = 5);
};