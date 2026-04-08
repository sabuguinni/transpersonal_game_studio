#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "CharacterArchetypeSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Protagonist"),
    TribalLeader       UMETA(DisplayName = "Tribal Leader"),
    TribalWarrior      UMETA(DisplayName = "Tribal Warrior"),
    TribalShaman       UMETA(DisplayName = "Tribal Shaman"),
    TribalHunter       UMETA(DisplayName = "Tribal Hunter"),
    TribalCrafter      UMETA(DisplayName = "Tribal Crafter"),
    TribalElder        UMETA(DisplayName = "Tribal Elder"),
    TribalChild        UMETA(DisplayName = "Tribal Child"),
    LostSurvivor       UMETA(DisplayName = "Lost Survivor"),
    WildHermit         UMETA(DisplayName = "Wild Hermit"),
    TraderNomad        UMETA(DisplayName = "Trader Nomad"),
    HostileRaider      UMETA(DisplayName = "Hostile Raider"),
    QuestGiver         UMETA(DisplayName = "Quest Giver"),
    Merchant           UMETA(DisplayName = "Merchant"),
    Guide              UMETA(DisplayName = "Guide")
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male               UMETA(DisplayName = "Male"),
    Female             UMETA(DisplayName = "Female"),
    Random             UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class ECharacterAgeGroup : uint8
{
    Young              UMETA(DisplayName = "Young (18-25)"),
    Adult              UMETA(DisplayName = "Adult (26-45)"),
    MiddleAged         UMETA(DisplayName = "Middle Aged (46-60)"),
    Elder              UMETA(DisplayName = "Elder (60+)")
};

UENUM(BlueprintType)
enum class ECharacterBuild : uint8
{
    Slim               UMETA(DisplayName = "Slim"),
    Athletic           UMETA(DisplayName = "Athletic"),
    Muscular           UMETA(DisplayName = "Muscular"),
    Heavy              UMETA(DisplayName = "Heavy"),
    Frail              UMETA(DisplayName = "Frail")
};

UENUM(BlueprintType)
enum class ESkinTone : uint8
{
    VeryLight          UMETA(DisplayName = "Very Light"),
    Light              UMETA(DisplayName = "Light"),
    Medium             UMETA(DisplayName = "Medium"),
    Dark               UMETA(DisplayName = "Dark"),
    VeryDark           UMETA(DisplayName = "Very Dark")
};

USTRUCT(BlueprintType)
struct FCharacterPhysicalTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    ECharacterGender Gender = ECharacterGender::Random;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    ECharacterAgeGroup AgeGroup = ECharacterAgeGroup::Adult;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    ECharacterBuild Build = ECharacterBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    ESkinTone SkinTone = ESkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HeightScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FString HairStyle = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    FLinearColor EyeColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    TArray<FString> FacialFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    TArray<FString> Scars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    TArray<FString> Tattoos;
};

USTRUCT(BlueprintType)
struct FCharacterClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString OutfitStyle = TEXT("Primitive");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearLevel = 0.5f; // 0 = new, 1 = very worn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> Weapons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> Tools;
};

USTRUCT(BlueprintType)
struct FCharacterPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FString PersonalityType = TEXT("Neutral");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Aggressiveness = 0.5f; // 0 = peaceful, 1 = very aggressive

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Trustworthiness = 0.5f; // 0 = untrustworthy, 1 = very trustworthy

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Intelligence = 0.5f; // 0 = simple, 1 = very intelligent

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Sociability = 0.5f; // 0 = antisocial, 1 = very social

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Quirks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Fears;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    TArray<FString> Motivations;
};

USTRUCT(BlueprintType)
struct FCharacterArchetypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    ECharacterArchetype Archetype = ECharacterArchetype::LostSurvivor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName = TEXT("Lost Survivor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString Description = TEXT("A survivor lost in the prehistoric world");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    FCharacterPhysicalTraits PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FCharacterClothing Clothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FCharacterPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TArray<FString> PossibleDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TArray<FString> PossibleQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    float HostilityLevel = 0.0f; // 0 = friendly, 1 = immediately hostile

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bCanBeTamed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    bool bIsEssentialNPC = false;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Archetypes")
    UDataTable* ArchetypeDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<TSoftObjectPtr<class UMetaHumanIdentity>> MalePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<TSoftObjectPtr<class UMetaHumanIdentity>> FemalePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Assets")
    TArray<TSoftObjectPtr<class USkeletalMesh>> PrimitiveClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Assets")
    TArray<TSoftObjectPtr<class USkeletalMesh>> ModernClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair Assets")
    TArray<TSoftObjectPtr<class UGroomAsset>> HairStyles;

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterArchetypeData GetArchetypeData(ECharacterArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterArchetypeData GenerateRandomCharacter(ECharacterArchetype Archetype) const;
};