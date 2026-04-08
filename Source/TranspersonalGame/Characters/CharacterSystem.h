#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "MetaHumanCreator/Public/MetaHumanCharacter.h"
#include "CharacterSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalShaman    UMETA(DisplayName = "Tribal Shaman"),
    TribalCrafter   UMETA(DisplayName = "Tribal Crafter"),
    TribalChild     UMETA(DisplayName = "Tribal Child"),
    TribalNomad     UMETA(DisplayName = "Tribal Nomad")
};

UENUM(BlueprintType)
enum class ETribalCulture : uint8
{
    RiverPeople     UMETA(DisplayName = "River People"),
    MountainClans   UMETA(DisplayName = "Mountain Clans"),
    ForestDwellers  UMETA(DisplayName = "Forest Dwellers"),
    PlainHunters    UMETA(DisplayName = "Plain Hunters"),
    CaveClans       UMETA(DisplayName = "Cave Clans")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterVariationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FacialStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HairVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ScarPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TattooPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ClothingSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AccessorySet;

    FCharacterVariationData()
    {
        SkinTone = 0.5f;
        BodyBuild = 0.5f;
        FacialStructure = 0.5f;
        HairVariation = 0.5f;
        ScarPattern = 0.0f;
        TattooPattern = 0.0f;
        ClothingSet = 0;
        AccessorySet = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETribalCulture Culture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FCharacterVariationData VisualVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    FCharacterProfile()
    {
        CharacterName = TEXT("Unnamed Character");
        Archetype = ECharacterArchetype::TribalHunter;
        Culture = ETribalCulture::ForestDwellers;
        BackgroundStory = TEXT("");
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterDatabase : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TArray<FCharacterProfile> CharacterProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation Settings")
    int32 MaxVariationsPerArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variation Settings")
    float MinimumVariationDistance;

    UCharacterDatabase()
    {
        MaxVariationsPerArchetype = 50;
        MinimumVariationDistance = 0.3f;
    }

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterProfile GenerateRandomCharacter(ECharacterArchetype Archetype, ETribalCulture Culture);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    TArray<FCharacterProfile> GetCharactersByArchetype(ECharacterArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    TArray<FCharacterProfile> GetCharactersByCulture(ETribalCulture Culture);
};