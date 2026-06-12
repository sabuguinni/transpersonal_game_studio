#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_MetaHumanManager.generated.h"

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    TribalWarrior    UMETA(DisplayName = "Tribal Warrior"),
    TribalHunter     UMETA(DisplayName = "Tribal Hunter"),
    TribalShaman     UMETA(DisplayName = "Tribal Shaman"),
    SurvivorMale     UMETA(DisplayName = "Survivor Male"),
    SurvivorFemale   UMETA(DisplayName = "Survivor Female"),
    ElderWise        UMETA(DisplayName = "Elder Wise"),
    YoungScout       UMETA(DisplayName = "Young Scout")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Tanned"),
    Weathered   UMETA(DisplayName = "Weathered")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_CharacterType CharacterType = EChar_CharacterType::TribalWarrior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<USkeletalMesh> HeadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    FChar_CharacterPreset()
    {
        CharacterType = EChar_CharacterType::TribalWarrior;
        SkinTone = EChar_SkinTone::Medium;
        CharacterName = TEXT("Unnamed");
    }
};

/**
 * MetaHuman Manager for creating and customizing prehistoric survival characters
 * Handles character presets, appearance variations, and MetaHuman integration
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    // Character Presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Presets")
    TArray<FChar_CharacterPreset> CharacterPresets;

    // Default MetaHuman Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TSoftObjectPtr<USkeletalMesh> DefaultMaleBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TSoftObjectPtr<USkeletalMesh> DefaultFemaleBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> MaleHeadVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> FemaleHeadVariations;

    // Skin Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    // Character Creation Functions
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterPreset CreateRandomCharacter(EChar_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    bool ApplyCharacterPreset(USkeletalMeshComponent* TargetMesh, const FChar_CharacterPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void InitializeDefaultPresets();

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    FChar_CharacterPreset GetPresetByType(EChar_CharacterType CharacterType);

    // Appearance Customization
    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool SetCharacterSkinTone(USkeletalMeshComponent* TargetMesh, EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool SetCharacterClothing(USkeletalMeshComponent* TargetMesh, int32 ClothingIndex);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FString> GetAvailableCharacterNames();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetPresetCount() const { return CharacterPresets.Num(); }

private:
    void LoadDefaultAssets();
    UMaterialInterface* GetSkinMaterialForTone(EChar_SkinTone SkinTone);
    USkeletalMesh* GetRandomHeadMesh(bool bIsFemale);
};