#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DataTable.h"
#include "Char_MetaHumanManager.generated.h"

// Character appearance data structure
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    FChar_AppearanceData()
    {
        CharacterName = TEXT("Primitive Human");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        BodyMass = 1.0f;
        Height = 1.0f;
    }
};

// Character archetype for different roles
UENUM(BlueprintType)
enum class EChar_Archetype : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child")
};

// Clothing style for Cretaceous period
UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    PlantFiber      UMETA(DisplayName = "Plant Fiber"),
    BoneArmor       UMETA(DisplayName = "Bone Armor"),
    Minimal         UMETA(DisplayName = "Minimal"),
    Ceremonial      UMETA(DisplayName = "Ceremonial")
};

/**
 * MetaHuman Manager for creating and customizing prehistoric human characters
 * Handles character appearance, clothing, and accessories for Cretaceous period
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    // Character appearance database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    TSoftObjectPtr<UDataTable> CharacterAppearanceTable;

    // Available character archetypes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
    TArray<EChar_Archetype> AvailableArchetypes;

    // Default appearance data for each archetype
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Creation")
    TMap<EChar_Archetype, FChar_AppearanceData> ArchetypeDefaults;

    // Material parameter collections for dynamic customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<class UMaterialParameterCollection> SkinParameterCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<class UMaterialParameterCollection> ClothingParameterCollection;

public:
    // Create a new character with specified archetype
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class ATranspersonalCharacter* CreateCharacterFromArchetype(EChar_Archetype Archetype, FVector SpawnLocation, FRotator SpawnRotation);

    // Customize existing character appearance
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearanceData(class ATranspersonalCharacter* Character, const FChar_AppearanceData& AppearanceData);

    // Generate random appearance within archetype constraints
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_AppearanceData GenerateRandomAppearance(EChar_Archetype Archetype);

    // Apply clothing style to character
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyClothingStyle(class ATranspersonalCharacter* Character, EChar_ClothingStyle ClothingStyle);

    // Add accessories to character (jewelry, weapons, tools)
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddCharacterAccessories(class ATranspersonalCharacter* Character, const TArray<TSoftObjectPtr<UStaticMesh>>& Accessories);

    // Update skin tone and body proportions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void UpdatePhysicalTraits(class ATranspersonalCharacter* Character, FLinearColor SkinTone, float BodyMass, float Height);

    // Get appearance data for character
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Character Info")
    FChar_AppearanceData GetCharacterAppearance(class ATranspersonalCharacter* Character);

    // Validate character setup
    UFUNCTION(BlueprintCallable, Category = "Character Validation")
    bool ValidateCharacterSetup(class ATranspersonalCharacter* Character);

protected:
    // Initialize archetype defaults
    void InitializeArchetypeDefaults();

    // Load appearance data from data table
    FChar_AppearanceData* GetAppearanceDataFromTable(const FString& CharacterName);

    // Apply material parameters
    void ApplyMaterialParameters(class USkeletalMeshComponent* MeshComponent, const FChar_AppearanceData& AppearanceData);

private:
    // Cache for loaded assets
    TMap<FString, TSoftObjectPtr<USkeletalMesh>> LoadedMeshCache;
    TMap<FString, TSoftObjectPtr<UMaterialInterface>> LoadedMaterialCache;
};