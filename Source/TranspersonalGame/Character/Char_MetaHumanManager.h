#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanManager.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair = 0,
    Medium,
    Olive,
    Tan,
    Dark
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean = 0,
    Average,
    Muscular,
    Heavy
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    LeatherWraps = 0,
    FurCloak,
    PlantFiber,
    AnimalHide,
    Minimal
};

USTRUCT(BlueprintType)
struct FChar_CharacterPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float ScarIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel;

    FChar_CharacterPreset()
    {
        PresetName = TEXT("Default");
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Average;
        ClothingStyle = EChar_ClothingStyle::LeatherWraps;
        ScarIntensity = 0.3f;
        WeatheringLevel = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Presets")
    TArray<FChar_CharacterPreset> AvailablePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> MaleCharacterMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> FemaleCharacterMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyCharacterPreset(class ATranspersonalCharacter* Character, const FChar_CharacterPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void RandomizeCharacterAppearance(class ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterPreset GetRandomPreset() const;

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void SetCharacterSkinTone(class ATranspersonalCharacter* Character, EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void SetCharacterClothing(class ATranspersonalCharacter* Character, EChar_ClothingStyle ClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyWeatheringEffects(class ATranspersonalCharacter* Character, float WeatheringLevel);

private:
    void InitializeDefaultPresets();
    UMaterialInterface* GetSkinMaterialForTone(EChar_SkinTone SkinTone) const;
    UMaterialInterface* GetClothingMaterialForStyle(EChar_ClothingStyle ClothingStyle) const;
};