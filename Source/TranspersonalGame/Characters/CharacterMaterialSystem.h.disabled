#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataAsset.h"
#include "JurassicCharacterArtist.h"
#include "CharacterMaterialSystem.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class ACharacter;
class USkeletalMeshComponent;

/**
 * Enum for clothing types used in material assignment
 */
UENUM(BlueprintType)
enum class EClothingType : uint8
{
    Shirt           UMETA(DisplayName = "Shirt/Top"),
    Pants           UMETA(DisplayName = "Pants/Bottom"),
    Shoes           UMETA(DisplayName = "Shoes"),
    Jacket          UMETA(DisplayName = "Jacket/Outerwear"),
    Accessories     UMETA(DisplayName = "Accessories"),
    
    MAX             UMETA(Hidden)
};

/**
 * Character Material System
 * 
 * Manages all material creation and application for MetaHuman characters.
 * Handles skin tones, clothing wear, environmental effects, and visual storytelling through materials.
 * Every material tells part of the character's story - from sun damage to battle scars.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCharacterMaterialSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCharacterMaterialSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core material creation functions
    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    UMaterialInstanceDynamic* CreateSkinMaterial(ESkinTone SkinTone, const FCharacterVisualStory& VisualStory);

    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    UMaterialInstanceDynamic* CreateClothingMaterial(EClothingType ClothingType, const FCharacterVisualStory& VisualStory);

    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    UMaterialInstanceDynamic* CreateHairMaterial(const FLinearColor& HairColor, const FCharacterVisualStory& VisualStory);

    // Character material application
    UFUNCTION(BlueprintCallable, Category = "Character Materials")
    void ApplyCharacterMaterials(ACharacter* Character, const FMetaHumanCustomization& Customization);

    // Visual storytelling through materials
    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplySurvivalEffectsToSkin(UMaterialInstanceDynamic* SkinMaterial, const FCharacterVisualStory& VisualStory);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyClothingWear(UMaterialInstanceDynamic* ClothingMaterial, const FCharacterVisualStory& VisualStory);

    UFUNCTION(BlueprintCallable, Category = "Visual Storytelling")
    void ApplyHairEnvironmentalEffects(UMaterialInstanceDynamic* HairMaterial, const FCharacterVisualStory& VisualStory);

    // Dynamic material updates
    UFUNCTION(BlueprintCallable, Category = "Dynamic Materials")
    void UpdateMaterialForTimeOfDay(ACharacter* Character, float TimeOfDay);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    UMaterialInterface* FindClosestHairMaterial(const FLinearColor& TargetColor);

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    float CalculateLightingMultiplier(float TimeOfDay);

protected:
    // Material loading and initialization
    void LoadBaseMaterials();
    void LoadSkinMaterials();
    void LoadClothingMaterials();
    void LoadHairMaterials();
    void InitializeMaterialParameters();

    // Internal material application
    void ApplyClothingMaterials(ACharacter* Character, const FCharacterVisualStory& VisualStory);

protected:
    // Base material collections
    UPROPERTY()
    TMap<ESkinTone, UMaterialInterface*> SkinMaterials;

    UPROPERTY()
    TArray<UMaterialInterface*> ClothingMaterials;

    UPROPERTY()
    TMap<FLinearColor, UMaterialInterface*> HairMaterials;

    // Dynamic material cache
    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> CachedDynamicMaterials;

    // Material parameter names for consistency
    UPROPERTY()
    TArray<FString> SkinParameterNames;

    UPROPERTY()
    TArray<FString> ClothingParameterNames;

    // Default material values
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Defaults")
    float DefaultSkinRoughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Defaults")
    float DefaultSkinSpecular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Defaults")
    float DefaultSkinSubsurface;

    // System state
    UPROPERTY()
    bool bMaterialsInitialized;
};