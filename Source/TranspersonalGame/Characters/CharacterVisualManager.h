#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterVisualManager.generated.h"

UENUM(BlueprintType)
enum class EChar_CharacterType : uint8
{
    Player          UMETA(DisplayName = "Player Character"),
    TribalWarrior   UMETA(DisplayName = "Tribal Warrior"),
    TribalElder     UMETA(DisplayName = "Tribal Elder"),
    TribalShaman    UMETA(DisplayName = "Tribal Shaman"),
    TribalScout     UMETA(DisplayName = "Tribal Scout"),
    TribalCrafter   UMETA(DisplayName = "Tribal Crafter")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterVisualData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FString CharacterName;

    FChar_CharacterVisualData()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        BodyScale = 1.0f;
        CharacterName = TEXT("Unnamed");
    }
};

/**
 * Component responsible for managing character visual appearance
 * Handles mesh assignment, material customization, and visual variations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterVisualManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterVisualManager();

protected:
    virtual void BeginPlay() override;

public:
    // Character type and visual data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_CharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_CharacterVisualData VisualData;

    // Preset visual configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EChar_CharacterType, FChar_CharacterVisualData> CharacterPresets;

    // Runtime customization
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void ApplyCharacterType(EChar_CharacterType NewType);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetCustomVisualData(const FChar_CharacterVisualData& NewVisualData);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetBodyScale(float NewScale);

    // Mesh and material management
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void UpdateCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void UpdateMaterials();

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Character Visual")
    void ApplyTribalWarriorPreset();

    UFUNCTION(CallInEditor, Category = "Character Visual")
    void ApplyTribalElderPreset();

    UFUNCTION(CallInEditor, Category = "Character Visual")
    void ApplyTribalShamanPreset();

    UFUNCTION(CallInEditor, Category = "Character Visual")
    void RefreshVisuals();

protected:
    // Initialize default presets
    void InitializeCharacterPresets();

    // Get reference to character's skeletal mesh component
    class USkeletalMeshComponent* GetCharacterMesh() const;

    // Apply material parameters
    void ApplyMaterialParameters(UMaterialInterface* Material, const FChar_CharacterVisualData& Data);

    // Generate random variations
    FLinearColor GenerateRandomSkinTone() const;
    FLinearColor GenerateRandomHairColor() const;
    float GenerateRandomBodyScale() const;

private:
    // Cache for performance
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CachedMeshComponent;

    // Material instances for runtime modification
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicClothingMaterial;
};