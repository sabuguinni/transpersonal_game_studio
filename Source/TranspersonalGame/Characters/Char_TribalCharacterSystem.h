#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Char_TribalCharacterSystem.generated.h"

// Tribal character body types for Cretaceous period survivors
UENUM(BlueprintType)
enum class EChar_TribalBodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Athletic    UMETA(DisplayName = "Athletic Gatherer"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Crafter")
};

// Tribal clothing sets based on role and materials available in Cretaceous period
UENUM(BlueprintType)
enum class EChar_TribalClothing : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Leathers"),
    Gatherer    UMETA(DisplayName = "Gatherer Wraps"),
    Warrior     UMETA(DisplayName = "Warrior Armor"),
    Shaman      UMETA(DisplayName = "Ritual Garb")
};

// Skin variations for different tribal groups
UENUM(BlueprintType)
enum class EChar_SkinVariation : uint8
{
    PaleNorthern    UMETA(DisplayName = "Pale Northern"),
    TannedPlains    UMETA(DisplayName = "Tanned Plains"),
    BronzeDesert    UMETA(DisplayName = "Bronze Desert"),
    DarkForest      UMETA(DisplayName = "Dark Forest"),
    WeatheredCoast  UMETA(DisplayName = "Weathered Coast"),
    AshenVolcanic   UMETA(DisplayName = "Ashen Volcanic")
};

// Data structure for tribal character configuration
USTRUCT(BlueprintType)
struct FChar_TribalCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalBodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_SkinVariation SkinVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalClothing ClothingSet;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    FChar_TribalCharacterData()
    {
        CharacterName = TEXT("Tribal_Character");
        BodyType = EChar_TribalBodyType::Athletic;
        SkinVariation = EChar_SkinVariation::TannedPlains;
        ClothingSet = EChar_TribalClothing::Hunter;
    }
};

/**
 * Tribal Character System for Cretaceous period human survivors
 * Manages appearance, clothing, and accessories for primitive human characters
 * Based on realistic anthropological research of prehistoric human societies
 */
UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_TribalCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_TribalCharacter();

protected:
    virtual void BeginPlay() override;

    // Character customization data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character", meta = (AllowPrivateAccess = "true"))
    FChar_TribalCharacterData CharacterData;

    // Data table containing all tribal character configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    TSoftObjectPtr<UDataTable> TribalCharacterDataTable;

    // Accessory attachment points for tools and decorations
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tribal Character")
    TArray<UStaticMeshComponent*> AccessoryComponents;

    // Dynamic material instances for runtime customization
    UPROPERTY(BlueprintReadOnly, Category = "Tribal Character")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

public:
    // Apply character configuration from data table
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ApplyTribalConfiguration(const FString& ConfigurationName);

    // Set body type and update mesh accordingly
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetBodyType(EChar_TribalBodyType NewBodyType);

    // Set skin variation and update materials
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetSkinVariation(EChar_SkinVariation NewSkinVariation);

    // Set clothing set and update materials
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void SetClothingSet(EChar_TribalClothing NewClothingSet);

    // Add accessory mesh to character (tools, jewelry, etc.)
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    UStaticMeshComponent* AddAccessory(UStaticMesh* AccessoryMesh, const FName& SocketName);

    // Remove all accessories
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void ClearAccessories();

    // Get current character configuration
    UFUNCTION(BlueprintPure, Category = "Tribal Character")
    FChar_TribalCharacterData GetCharacterData() const { return CharacterData; }

    // Randomize character appearance within tribal constraints
    UFUNCTION(BlueprintCallable, Category = "Tribal Character")
    void RandomizeAppearance();

protected:
    // Internal methods for applying visual changes
    void UpdateCharacterMesh();
    void UpdateSkinMaterial();
    void UpdateClothingMaterial();
    void CreateDynamicMaterials();

    // Socket names for accessory attachment
    static const FName HeadAccessorySocket;
    static const FName BackAccessorySocket;
    static const FName BeltAccessorySocket;
    static const FName WristAccessorySocket;
};