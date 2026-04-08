#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataAsset.h"
#include "CharacterArtistSystem.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist - Paleontologist"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalHunter    UMETA(DisplayName = "Tribal Hunter"),
    TribalGatherer  UMETA(DisplayName = "Tribal Gatherer"),
    TribalShaman    UMETA(DisplayName = "Tribal Shaman"),
    TribalChild     UMETA(DisplayName = "Tribal Child"),
    Survivor        UMETA(DisplayName = "Lone Survivor"),
    Wanderer        UMETA(DisplayName = "Wanderer")
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female"),
    Random      UMETA(DisplayName = "Random")
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Child       UMETA(DisplayName = "Child (8-15)"),
    Young       UMETA(DisplayName = "Young Adult (16-25)"),
    Adult       UMETA(DisplayName = "Adult (26-45)"),
    Elder       UMETA(DisplayName = "Elder (46+)")
};

USTRUCT(BlueprintType)
struct FCharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float SkinTone = 0.5f; // 0.0 = Very Light, 1.0 = Very Dark

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float BodyBuild = 0.5f; // 0.0 = Lean, 1.0 = Muscular

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height = 0.5f; // 0.0 = Short, 1.0 = Tall

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear and Tear")
    float WearLevel = 0.3f; // 0.0 = Clean, 1.0 = Heavily Worn

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear and Tear")
    float ScarLevel = 0.2f; // 0.0 = No Scars, 1.0 = Many Scars

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wear and Tear")
    float DirtLevel = 0.4f; // 0.0 = Clean, 1.0 = Very Dirty

    FCharacterVariation()
    {
        SkinTone = FMath::RandRange(0.2f, 0.8f);
        BodyBuild = FMath::RandRange(0.3f, 0.7f);
        Height = FMath::RandRange(0.4f, 0.6f);
        WearLevel = FMath::RandRange(0.2f, 0.8f);
        ScarLevel = FMath::RandRange(0.0f, 0.5f);
        DirtLevel = FMath::RandRange(0.3f, 0.7f);
    }
};

USTRUCT(BlueprintType)
struct FCharacterDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterAge Age;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FCharacterVariation PhysicalVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> PersonalityTraits;
};

/**
 * Character Artist System
 * Manages the creation and variation of human characters using MetaHuman Creator
 * Ensures each character feels unique and fits the prehistoric survival setting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterArtistSystem : public UDataAsset
{
    GENERATED_BODY()

public:
    UCharacterArtistSystem();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TArray<FCharacterDefinition> CharacterDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    int32 MaxUniqueCharacters = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    bool bEnsureVisualDiversity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation Settings")
    float MinimumVariationThreshold = 0.3f;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterDefinition CreateCharacter(ECharacterArchetype Archetype, ECharacterGender Gender = ECharacterGender::Random);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterDefinition CreateProtagonist();

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    TArray<FCharacterDefinition> CreateTribalCommunity(int32 CommunitySize = 8);

    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateCharacterDiversity(const FCharacterDefinition& NewCharacter);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SaveCharacterToDatabase(const FCharacterDefinition& Character);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FCharacterDefinition GetCharacterByName(const FString& Name);

private:
    FCharacterVariation GenerateVariationForArchetype(ECharacterArchetype Archetype, ECharacterAge Age);
    FString GenerateCharacterName(ECharacterGender Gender, ECharacterArchetype Archetype);
    TArray<FString> GeneratePersonalityTraits(ECharacterArchetype Archetype);
    FText GenerateBackgroundStory(ECharacterArchetype Archetype, ECharacterGender Gender);
};