#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimBlueprint.h"
#include "MetaHumanCreator/Public/MetaHumanCharacter.h"
#include "CharacterSystem.generated.h"

/**
 * Core Character System for Transpersonal Game Studio
 * Handles MetaHuman character creation, variation, and management
 * Supports procedural diversity and emotional expression systems
 */

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist         UMETA(DisplayName = "Dr. Protagonist"),
    TribalLeader       UMETA(DisplayName = "Tribal Leader"),
    Hunter             UMETA(DisplayName = "Hunter"),
    Gatherer           UMETA(DisplayName = "Gatherer"),
    Shaman             UMETA(DisplayName = "Shaman"),
    Child              UMETA(DisplayName = "Child"),
    Elder              UMETA(DisplayName = "Elder"),
    Warrior            UMETA(DisplayName = "Warrior"),
    Crafter            UMETA(DisplayName = "Crafter"),
    Scout              UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Neutral            UMETA(DisplayName = "Neutral"),
    Fear               UMETA(DisplayName = "Fear"),
    Curiosity          UMETA(DisplayName = "Curiosity"),
    Determination      UMETA(DisplayName = "Determination"),
    Exhaustion         UMETA(DisplayName = "Exhaustion"),
    Hope               UMETA(DisplayName = "Hope"),
    Despair            UMETA(DisplayName = "Despair"),
    Vigilance          UMETA(DisplayName = "Vigilance"),
    Wonder             UMETA(DisplayName = "Wonder"),
    Survival           UMETA(DisplayName = "Survival Mode")
};

USTRUCT(BlueprintType)
struct FCharacterVariationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float HeightVariation = 0.0f; // -1.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float BuildVariation = 0.0f; // -1.0 (lean) to 1.0 (robust)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float AgeVariation = 0.0f; // 0.0 (young) to 1.0 (old)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float FaceWidth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float EyeSize = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float NoseShape = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float JawDefinition = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin & Scars")
    float SkinWeathering = 0.0f; // 0.0 (smooth) to 1.0 (weathered)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin & Scars")
    TArray<FVector2D> ScarLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin & Scars")
    float SkinTone = 0.0f; // Procedural skin tone variation

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 HairStyleIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    int32 ClothingSetIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float ClothingWear = 0.0f; // 0.0 (new) to 1.0 (tattered)
};

USTRUCT(BlueprintType)
struct FCharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    ECharacterArchetype Archetype = ECharacterArchetype::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FCharacterVariationData PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EEmotionalState DefaultEmotionalState = EEmotionalState::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PersonalityTraits[5]; // Big Five: Openness, Conscientiousness, Extraversion, Agreeableness, Neuroticism

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    int32 LODLevel = 0; // 0 = Highest quality, 3 = Lowest quality

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Technical")
    bool bUseNaniteGeometry = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCharacterSystem : public UObject
{
    GENERATED_BODY()

public:
    UCharacterSystem();

    // Character Creation
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FCharacterProfile GenerateRandomCharacter(ECharacterArchetype ArchetypeHint = ECharacterArchetype::Gatherer);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    USkeletalMeshComponent* CreateMetaHumanFromProfile(const FCharacterProfile& Profile, AActor* OwnerActor);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyEmotionalState(USkeletalMeshComponent* Character, EEmotionalState EmotionalState, float Intensity = 1.0f);

    // Character Variation
    UFUNCTION(BlueprintCallable, Category = "Character Variation")
    FCharacterVariationData GenerateVariationData(ECharacterArchetype Archetype, int32 Seed = -1);

    UFUNCTION(BlueprintCallable, Category = "Character Variation")
    void ApplyVariationToMetaHuman(USkeletalMeshComponent* MetaHuman, const FCharacterVariationData& VariationData);

    // Character Database
    UFUNCTION(BlueprintCallable, Category = "Character Database")
    void RegisterCharacterProfile(const FCharacterProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Character Database")
    FCharacterProfile GetCharacterProfile(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Character Database")
    TArray<FCharacterProfile> GetCharactersByArchetype(ECharacterArchetype Archetype);

    // Performance & LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCharacterLOD(USkeletalMeshComponent* Character, int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableNaniteGeometry(USkeletalMeshComponent* Character, bool bEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Database")
    TMap<FString, FCharacterProfile> CharacterDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> BaseMetaHumanMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> HairMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> ClothingMeshes;

private:
    // Internal character generation logic
    FCharacterVariationData GenerateArchetypeSpecificVariation(ECharacterArchetype Archetype, int32 Seed);
    void ApplyProceduralAging(FCharacterVariationData& VariationData, float AgeValue);
    void ApplyEnvironmentalWeathering(FCharacterVariationData& VariationData, float Severity);
    FLinearColor GenerateRealisticSkinTone(int32 Seed);
    FLinearColor GenerateRealisticHairColor(float AgeValue, int32 Seed);
};