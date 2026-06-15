#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_PrimitiveHumanSystem.generated.h"

// Character appearance variation data
USTRUCT(BlueprintType)
struct FChar_AppearanceVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyMassScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 TattooPattern;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 ClothingStyle;

    FChar_AppearanceVariation()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyMassScale = 1.0f;
        HeightScale = 1.0f;
        TattooPattern = 0;
        ClothingStyle = 0;
    }
};

// Equipment and tool data
USTRUCT(BlueprintType)
struct FChar_EquipmentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* WeaponMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* ToolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* AccessoryMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString EquipmentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    float DurabilityPercent;

    FChar_EquipmentData()
    {
        WeaponMesh = nullptr;
        ToolMesh = nullptr;
        AccessoryMesh = nullptr;
        EquipmentName = TEXT("Basic Tools");
        DurabilityPercent = 100.0f;
    }
};

// Character archetype definitions
UENUM(BlueprintType)
enum class EChar_HumanArchetype : uint8
{
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Elder           UMETA(DisplayName = "Elder"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Healer          UMETA(DisplayName = "Healer"),
    Child           UMETA(DisplayName = "Child")
};

/**
 * Primitive Human Character System
 * Manages appearance, equipment, and behavioral traits for prehistoric human characters
 * Supports diverse character generation with realistic survival-focused archetypes
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Character appearance system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceVariation AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    EChar_HumanArchetype CharacterArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_EquipmentData EquipmentData;

    // Dynamic material instances for customization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    class UMaterialInstanceDynamic* SkinMaterialInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    class UMaterialInstanceDynamic* ClothingMaterialInstance;

    // Equipment mesh components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* WeaponMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* ToolMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    class UStaticMeshComponent* AccessoryMeshComponent;

    // Character behavior traits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float CourageLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float CraftingSkill;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float HuntingSkill;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float SurvivalInstinct;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Traits")
    float SocialStanding;

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearanceVariation(const FChar_AppearanceVariation& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetCharacterArchetype(EChar_HumanArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipWeapon(class UStaticMesh* WeaponMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipTool(class UStaticMesh* ToolMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipAccessory(class UStaticMesh* AccessoryMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void UpdateEquipmentDurability(float NewDurabilityPercent);

    // Archetype-specific behavior functions
    UFUNCTION(BlueprintCallable, Category = "Character Behavior")
    void ApplyArchetypeTraits();

    UFUNCTION(BlueprintCallable, Category = "Character Behavior")
    FString GetArchetypeDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character Behavior")
    TArray<FString> GetArchetypeSkills() const;

    // Character generation utilities
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static FChar_AppearanceVariation GenerateRandomAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    static EChar_HumanArchetype GenerateRandomArchetype();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void RandomizeCharacter();

    // Material and visual updates
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateSkinMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateClothingMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void UpdateTattooPattern();

    // Character interaction and dialogue
    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    FString GetGreetingDialogue() const;

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    FString GetContextualDialogue(const FString& Situation) const;

    UFUNCTION(BlueprintCallable, Category = "Character Interaction")
    bool CanInteractWith(AActor* OtherActor) const;

private:
    // Internal appearance management
    void InitializeAppearanceComponents();
    void SetupDefaultMaterials();
    void ApplyArchetypeDefaults();
    void UpdateEquipmentVisibility();

    // Archetype data tables
    UPROPERTY()
    class UDataTable* ArchetypeDataTable;

    UPROPERTY()
    class UDataTable* AppearanceDataTable;

    // Default materials
    UPROPERTY()
    class UMaterialInterface* DefaultSkinMaterial;

    UPROPERTY()
    class UMaterialInterface* DefaultClothingMaterial;
};

/**
 * Character Customization Manager
 * Handles batch character generation and appearance management
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_CharacterCustomizationManager : public UObject
{
    GENERATED_BODY()

public:
    UChar_CharacterCustomizationManager();

    // Batch character generation
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    TArray<AChar_PrimitiveHumanCharacter*> GenerateTribalGroup(int32 GroupSize, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    AChar_PrimitiveHumanCharacter* GenerateArchetypeCharacter(EChar_HumanArchetype DesiredArchetype, const FVector& SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void ApplyTribalTheme(TArray<AChar_PrimitiveHumanCharacter*>& Characters, const FString& TribalTheme);

    // Appearance presets
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void LoadAppearancePresets();

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FChar_AppearanceVariation GetPresetAppearance(const FString& PresetName);

    // Character diversity management
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void EnsureCharacterDiversity(TArray<AChar_PrimitiveHumanCharacter*>& Characters);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    bool ValidateCharacterUniqueness(const TArray<AChar_PrimitiveHumanCharacter*>& Characters);

private:
    UPROPERTY()
    TMap<FString, FChar_AppearanceVariation> AppearancePresets;

    UPROPERTY()
    TArray<EChar_HumanArchetype> AvailableArchetypes;
};