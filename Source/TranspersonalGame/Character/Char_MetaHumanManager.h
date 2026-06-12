#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float WeatheringIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float ScarIntensity;

    FChar_CharacterPreset()
    {
        PresetName = TEXT("Default");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        WeatheringIntensity = 0.5f;
        ScarIntensity = 0.3f;
    }
};

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    Player,
    TribalNPC,
    ElderNPC,
    WarriorNPC,
    ShamanNPC,
    ChildNPC
};

/**
 * MetaHuman Manager - Handles character creation and customization for prehistoric humans
 * Manages character presets, material variations, and visual diversity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_MetaHumanManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UChar_MetaHumanManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Character creation and customization
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    FChar_CharacterPreset CreateRandomCharacterPreset(EChar_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyCharacterPreset(class ACharacter* Character, const FChar_CharacterPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyWeatheringEffects(class USkeletalMeshComponent* MeshComponent, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplySurvivalScars(class USkeletalMeshComponent* MeshComponent, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    TArray<FChar_CharacterPreset> GetAvailablePresets() const { return CharacterPresets; }

    // Material and texture management
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    UMaterialInstanceDynamic* CreateCustomSkinMaterial(const FLinearColor& SkinTone, float Weathering);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    UMaterialInstanceDynamic* CreateCustomHairMaterial(const FLinearColor& HairColor);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void LoadCharacterAssets();

protected:
    // Character presets database
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaHuman")
    TArray<FChar_CharacterPreset> CharacterPresets;

    // Base materials for customization
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseHairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseClothingMaterial;

    // Skin tone variations for diversity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Diversity")
    TArray<FLinearColor> SkinToneVariations;

    // Hair color variations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Diversity")
    TArray<FLinearColor> HairColorVariations;

private:
    void InitializeDefaultPresets();
    void InitializeSkinToneVariations();
    void InitializeHairColorVariations();
    
    FChar_CharacterPreset GeneratePresetForType(EChar_CharacterType CharacterType);
    FLinearColor GetRandomSkinTone() const;
    FLinearColor GetRandomHairColor() const;
};