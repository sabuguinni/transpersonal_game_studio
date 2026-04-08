// CharacterArchetypes.h
// Transpersonal Game Studio - Character Art System
// Defines base archetypes for procedural character generation

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterArchetypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // PROTAGONIST
    Paleontologist      UMETA(DisplayName = "Dr. Paleontologist"),
    
    // TRIBAL NPCS (if they exist in this world)
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    TribalHunter        UMETA(DisplayName = "Tribal Hunter"),
    TribalShaman        UMETA(DisplayName = "Tribal Shaman"),
    TribalCrafter       UMETA(DisplayName = "Tribal Crafter"),
    
    // TIME TRAVELERS (other modern humans)
    Scientist           UMETA(DisplayName = "Modern Scientist"),
    Explorer            UMETA(DisplayName = "Modern Explorer"),
    Survivor            UMETA(DisplayName = "Survivor"),
    
    // GENERIC VARIANTS
    Wanderer            UMETA(DisplayName = "Wanderer"),
    Hermit              UMETA(DisplayName = "Hermit"),
    Trader              UMETA(DisplayName = "Trader"),
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterBodyType : uint8
{
    // MASCULINE VARIANTS
    Athletic_M          UMETA(DisplayName = "Athletic Male"),
    Lean_M              UMETA(DisplayName = "Lean Male"),
    Stocky_M            UMETA(DisplayName = "Stocky Male"),
    Tall_M              UMETA(DisplayName = "Tall Male"),
    
    // FEMININE VARIANTS
    Athletic_F          UMETA(DisplayName = "Athletic Female"),
    Lean_F              UMETA(DisplayName = "Lean Female"),
    Curvy_F             UMETA(DisplayName = "Curvy Female"),
    Tall_F              UMETA(DisplayName = "Tall Female"),
    
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterAgeRange : uint8
{
    YoungAdult          UMETA(DisplayName = "Young Adult (20-30)"),
    Adult               UMETA(DisplayName = "Adult (30-50)"),
    Mature              UMETA(DisplayName = "Mature (50+)"),
    
    MAX                 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    // FACIAL FEATURES
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceWidth = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseSize = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MouthSize = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawStrength = 0.5f;
    
    // SKIN AND HAIR
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
    
    // BODY PROPORTIONS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float Height = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyMass = 1.0f;
    
    // WEAR AND TEAR (survival marks)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SkinDamage = 0.0f;  // Scars, sun damage, etc.
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClothingWear = 0.0f;  // How worn/damaged clothing is
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Dirtiness = 0.0f;  // How dirty/grimy the character is
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterArchetypeData : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    ECharacterArchetype Archetype;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FString CharacterName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character")
    FText Description;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    ECharacterBodyType DefaultBodyType;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    ECharacterAgeRange DefaultAgeRange;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    FCharacterVariationData BaseVariation;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variation")
    float VariationRange = 0.3f;  // How much variation is allowed from base
    
    // METAHUMAN REFERENCES
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> MetaHumanMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    TSoftObjectPtr<class UAnimBlueprint> AnimationBlueprint;
    
    // CLOTHING OPTIONS
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clothing")
    TArray<TSoftObjectPtr<class USkeletalMesh>> ClothingOptions;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<class UMaterialInstance>> SkinMaterials;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<TSoftObjectPtr<class UMaterialInstance>> ClothingMaterials;
};

// CHARACTER GENERATION UTILITY
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterGenerator : public UObject
{
    GENERATED_BODY()

public:
    // Generate a random character variation based on archetype
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FCharacterVariationData GenerateRandomVariation(
        const UCharacterArchetypeData* ArchetypeData,
        int32 Seed = -1
    );
    
    // Apply variation to a MetaHuman character
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static bool ApplyVariationToCharacter(
        class ACharacter* Character,
        const FCharacterVariationData& Variation
    );
    
    // Generate unique identifier for character (for recognition system)
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FString GenerateCharacterID(
        ECharacterArchetype Archetype,
        const FCharacterVariationData& Variation
    );
};