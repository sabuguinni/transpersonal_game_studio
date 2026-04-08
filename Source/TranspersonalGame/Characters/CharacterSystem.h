#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonist"),
    Survivor           UMETA(DisplayName = "Survivor"),
    TribalLeader       UMETA(DisplayName = "Tribal Leader"),
    TribalWarrior      UMETA(DisplayName = "Tribal Warrior"),
    TribalShaman       UMETA(DisplayName = "Tribal Shaman"),
    TribalChild        UMETA(DisplayName = "Tribal Child"),
    Wanderer           UMETA(DisplayName = "Wanderer"),
    Hunter             UMETA(DisplayName = "Hunter"),
    Gatherer           UMETA(DisplayName = "Gatherer"),
    Elder              UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male               UMETA(DisplayName = "Male"),
    Female             UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    African            UMETA(DisplayName = "African"),
    Asian              UMETA(DisplayName = "Asian"),
    Caucasian          UMETA(DisplayName = "Caucasian"),
    Hispanic           UMETA(DisplayName = "Hispanic"),
    MiddleEastern      UMETA(DisplayName = "Middle Eastern"),
    Indigenous         UMETA(DisplayName = "Indigenous"),
    Mixed              UMETA(DisplayName = "Mixed")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child              UMETA(DisplayName = "Child (8-12)"),
    Teenager           UMETA(DisplayName = "Teenager (13-17)"),
    YoungAdult         UMETA(DisplayName = "Young Adult (18-30)"),
    MiddleAged         UMETA(DisplayName = "Middle Aged (31-50)"),
    Older              UMETA(DisplayName = "Older (51-65)"),
    Elder              UMETA(DisplayName = "Elder (65+)")
};

USTRUCT(BlueprintType)
struct FCharacterPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 1.0f; // Multiplier for base height

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 1.0f; // Multiplier for base weight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float MuscleMass = 0.5f; // 0.0 = lean, 1.0 = very muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float BodyFat = 0.3f; // 0.0 = very lean, 1.0 = heavy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
};

USTRUCT(BlueprintType)
struct FCharacterClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> HeadGear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Torso;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Arms;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Legs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Feet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> Accessories;
};

USTRUCT(BlueprintType)
struct FCharacterDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ECharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ECharacterEthnicity Ethnicity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classification")
    ECharacterAge AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FCharacterPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FCharacterClothing Clothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstory")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsUnique = false; // True for named characters, false for generic NPCs

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    int32 SpawnWeight = 1; // Higher weight = more likely to spawn
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TArray<FCharacterDefinition> Characters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TSoftObjectPtr<UDataTable> CharacterDataTable;

    UFUNCTION(BlueprintCallable, Category = "Character Database")
    FCharacterDefinition GetCharacterByName(const FString& CharacterName) const;

    UFUNCTION(BlueprintCallable, Category = "Character Database")
    TArray<FCharacterDefinition> GetCharactersByArchetype(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Character Database")
    FCharacterDefinition GetRandomCharacterByArchetype(ECharacterArchetype Archetype) const;
};