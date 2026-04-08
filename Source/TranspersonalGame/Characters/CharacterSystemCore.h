#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "CharacterSystemCore.generated.h"

/**
 * Core character system for Jurassic Survival Game
 * Handles MetaHuman-based character creation with procedural variation
 * 
 * Design Philosophy:
 * - Every face tells a story before speaking a word
 * - No two NPCs should look identical (anti-clone principle)
 * - Appearance IS character biography
 * - Vulnerability and humanity in a hostile world
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    // Primary Character
    Protagonist_Paleontologist  UMETA(DisplayName = "Protagonist - Paleontologist"),
    
    // Survivor Archetypes (if any other humans exist in this world)
    Survivor_Veteran           UMETA(DisplayName = "Veteran Survivor"),
    Survivor_Newcomer          UMETA(DisplayName = "Newcomer Survivor"),
    Survivor_Child             UMETA(DisplayName = "Child Survivor"),
    Survivor_Elder             UMETA(DisplayName = "Elder Survivor"),
    
    // Temporal Anomaly Characters (other time-displaced people)
    TimeDisplaced_Scientist    UMETA(DisplayName = "Time-Displaced Scientist"),
    TimeDisplaced_Explorer     UMETA(DisplayName = "Time-Displaced Explorer"),
    TimeDisplaced_Soldier      UMETA(DisplayName = "Time-Displaced Soldier"),
    
    // Mysterious Figures (if the narrative includes them)
    Mysterious_Guardian        UMETA(DisplayName = "Mysterious Guardian"),
    Mysterious_Wanderer        UMETA(DisplayName = "Mysterious Wanderer"),
    
    MAX                        UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female"),
    NonBinary   UMETA(DisplayName = "Non-Binary"),
    MAX         UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterEthnicity : uint8
{
    Caucasian           UMETA(DisplayName = "Caucasian"),
    AfricanAmerican     UMETA(DisplayName = "African American"),
    Hispanic            UMETA(DisplayName = "Hispanic"),
    Asian               UMETA(DisplayName = "Asian"),
    MiddleEastern       UMETA(DisplayName = "Middle Eastern"),
    NativeAmerican      UMETA(DisplayName = "Native American"),
    Mixed               UMETA(DisplayName = "Mixed Heritage"),
    MAX                 UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterAge : uint8
{
    Young       UMETA(DisplayName = "Young (18-30)"),
    Adult       UMETA(DisplayName = "Adult (31-50)"),
    MiddleAged  UMETA(DisplayName = "Middle-Aged (51-65)"),
    Elder       UMETA(DisplayName = "Elder (65+)"),
    MAX         UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ECharacterCondition : uint8
{
    Pristine        UMETA(DisplayName = "Pristine - Just Arrived"),
    Weathered       UMETA(DisplayName = "Weathered - Weeks Surviving"),
    Hardened        UMETA(DisplayName = "Hardened - Months Surviving"),
    Scarred         UMETA(DisplayName = "Scarred - Years Surviving"),
    Broken          UMETA(DisplayName = "Broken - Traumatized"),
    MAX             UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    // MetaHuman Base Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> BaseMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> HeadMesh;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<class USkeletalMesh> BodyMesh;
    
    // Facial Variation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FaceWidth = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EyeSize = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NoseShape = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MouthWidth = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float JawDefinition = 0.5f;
    
    // Skin and Hair
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
    
    // Survival Wear and Condition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DirtLevel = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarLevel = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ClothingWear = 0.2f;
    
    // Clothing Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<TSoftObjectPtr<class USkeletalMesh>> ClothingMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<TSoftObjectPtr<class UMaterialInterface>> ClothingMaterials;

    FCharacterVariationData()
    {
        // Default constructor with reasonable survival game defaults
    }
};

USTRUCT(BlueprintType)
struct FCharacterDefinition
{
    GENERATED_BODY()

    // Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString BackgroundStory;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype = ECharacterArchetype::Protagonist_Paleontologist;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterGender Gender = ECharacterGender::Male;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterEthnicity Ethnicity = ECharacterEthnicity::Caucasian;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterAge AgeGroup = ECharacterAge::Adult;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterCondition SurvivalCondition = ECharacterCondition::Pristine;
    
    // Visual Variation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FCharacterVariationData VariationData;
    
    // Gameplay Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TSoftClassPtr<class ACharacter> CharacterBlueprintClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
    TSoftClassPtr<class UAnimInstance> AnimationBlueprint;
    
    // Unique Identifier for save/load and networking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System")
    FGuid CharacterID;

    FCharacterDefinition()
    {
        CharacterID = FGuid::NewGuid();
    }
};

/**
 * Character System Core - Manages MetaHuman-based character creation and variation
 * This system ensures no two NPCs look identical while maintaining performance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterSystemCore : public UObject
{
    GENERATED_BODY()

public:
    UCharacterSystemCore();

    // Character Generation
    UFUNCTION(BlueprintCallable, Category = "Character System")
    FCharacterDefinition GenerateRandomCharacter(ECharacterArchetype DesiredArchetype = ECharacterArchetype::Protagonist_Paleontologist);
    
    UFUNCTION(BlueprintCallable, Category = "Character System")
    FCharacterDefinition GenerateCharacterFromSeed(int32 Seed, ECharacterArchetype DesiredArchetype);
    
    UFUNCTION(BlueprintCallable, Category = "Character System")
    class ACharacter* SpawnCharacterFromDefinition(const FCharacterDefinition& CharacterDef, const FTransform& SpawnTransform, UWorld* World);
    
    // Character Variation
    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplyVariationToCharacter(class ACharacter* Character, const FCharacterVariationData& VariationData);
    
    UFUNCTION(BlueprintCallable, Category = "Character System")
    FCharacterVariationData GenerateProceduralVariation(ECharacterGender Gender, ECharacterEthnicity Ethnicity, ECharacterAge Age);
    
    // MetaHuman Integration
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool LoadMetaHumanPreset(const FString& PresetName, FCharacterVariationData& OutVariationData);
    
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    TArray<FString> GetAvailableMetaHumanPresets(ECharacterGender Gender = ECharacterGender::Male);
    
    // Character Database
    UFUNCTION(BlueprintCallable, Category = "Character Database")
    void SaveCharacterDefinition(const FCharacterDefinition& CharacterDef);
    
    UFUNCTION(BlueprintCallable, Category = "Character Database")
    bool LoadCharacterDefinition(const FGuid& CharacterID, FCharacterDefinition& OutCharacterDef);
    
    UFUNCTION(BlueprintCallable, Category = "Character Database")
    TArray<FCharacterDefinition> GetCharactersByArchetype(ECharacterArchetype Archetype);

protected:
    // MetaHuman Asset Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TMap<ECharacterGender, TArray<TSoftObjectPtr<class USkeletalMesh>>> MetaHumanBaseMeshes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TMap<ECharacterEthnicity, TArray<TSoftObjectPtr<class UMaterialInterface>>> SkinMaterials;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing Assets")
    TMap<ECharacterArchetype, TArray<TSoftObjectPtr<class USkeletalMesh>>> ArchetypeClothing;
    
    // Character Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TMap<FGuid, FCharacterDefinition> CharacterDatabase;
    
    // Variation Ranges for Procedural Generation
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation")
    float MinVariationRange = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Generation")
    float MaxVariationRange = 0.8f;

private:
    // Internal generation helpers
    FCharacterVariationData GenerateVariationForArchetype(ECharacterArchetype Archetype, ECharacterGender Gender, ECharacterEthnicity Ethnicity);
    FLinearColor GenerateEthnicSkinTone(ECharacterEthnicity Ethnicity);
    FLinearColor GenerateRealisticHairColor(ECharacterEthnicity Ethnicity);
    FLinearColor GenerateRealisticEyeColor(ECharacterEthnicity Ethnicity);
    
    // Ensure no duplicate appearances
    bool IsVariationUnique(const FCharacterVariationData& NewVariation, float SimilarityThreshold = 0.85f);
    
    // MetaHuman-specific helpers
    void ConfigureMetaHumanMesh(class USkeletalMeshComponent* MeshComponent, const FCharacterVariationData& VariationData);
    void ApplyFacialMorphTargets(class USkeletalMeshComponent* HeadMesh, const FCharacterVariationData& VariationData);
};