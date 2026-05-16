#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_Gender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_AgeGroup AgeGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> Scars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString ClothingStyle;

    FChar_TribalAppearanceData()
    {
        CharacterName = TEXT("Unknown");
        Gender = EChar_Gender::Male;
        AgeGroup = EChar_AgeGroup::Adult;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f); // Tanned skin
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f); // Brown eyes
        BodyMass = 1.0f;
        Height = 1.0f;
        ClothingStyle = TEXT("Hunter");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalArchetype
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    TArray<FString> PreferredClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    TArray<FString> TypicalTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    FLinearColor PreferredSkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Archetype")
    float PreferredBodyMass;

    FChar_TribalArchetype()
    {
        ArchetypeName = TEXT("Hunter");
        Description = TEXT("Skilled tracker and hunter of prehistoric beasts");
        PreferredClothing = { TEXT("Animal Hide"), TEXT("Bone Ornaments") };
        TypicalTools = { TEXT("Spear"), TEXT("Stone Knife"), TEXT("Bone Tools") };
        PreferredSkinTone = FLinearColor(0.7f, 0.5f, 0.3f, 1.0f);
        PreferredBodyMass = 1.1f; // Slightly muscular
    }
};

/**
 * Manager for creating and customizing tribal characters in the Cretaceous world
 * Handles character diversity, appearance variation, and archetype-based generation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCharacterManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character Generation
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalAppearanceData GenerateRandomTribalCharacter();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalAppearanceData GenerateCharacterFromArchetype(const FString& ArchetypeName);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyAppearanceToCharacter(AActor* Character, const FChar_TribalAppearanceData& AppearanceData);

    // Archetype Management
    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    void RegisterArchetype(const FChar_TribalArchetype& Archetype);

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    FChar_TribalArchetype GetArchetype(const FString& ArchetypeName) const;

    UFUNCTION(BlueprintCallable, Category = "Archetypes")
    TArray<FString> GetAvailableArchetypes() const;

    // Diversity System
    UFUNCTION(BlueprintCallable, Category = "Diversity")
    void EnsureCharacterDiversity(TArray<AActor*>& Characters);

    UFUNCTION(BlueprintCallable, Category = "Diversity")
    float CalculateDiversityScore(const TArray<FChar_TribalAppearanceData>& Characters) const;

    // Material and Mesh Management
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateCharacterMaterials(AActor* Character, const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetSkinMaterial(const FLinearColor& SkinTone) const;

    UFUNCTION(BlueprintCallable, Category = "Materials")
    UMaterialInterface* GetClothingMaterial(const FString& ClothingStyle) const;

protected:
    // Archetype Database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<FString, FChar_TribalArchetype> TribalArchetypes;

    // Material Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<FString, TSoftObjectPtr<USkeletalMesh>> CharacterMeshes;

    // Diversity Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    float MinDiversityScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    int32 MaxSimilarCharacters;

    // Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FLinearColor> AvailableSkinTones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FLinearColor> AvailableHairColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FLinearColor> AvailableEyeColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FString> AvailableTribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    TArray<FString> AvailableScarTypes;

private:
    // Helper Functions
    FLinearColor GenerateRandomSkinTone() const;
    FLinearColor GenerateRandomHairColor() const;
    FLinearColor GenerateRandomEyeColor() const;
    FString GenerateRandomTribalMarking() const;
    FString GenerateRandomScar() const;
    float GenerateRandomBodyMass() const;
    float GenerateRandomHeight() const;
    
    void InitializeDefaultArchetypes();
    void InitializeDefaultMaterials();
    void InitializeColorPalettes();
};