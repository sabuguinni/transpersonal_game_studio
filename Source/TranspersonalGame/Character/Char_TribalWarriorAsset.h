#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_TribalWarriorAsset.generated.h"

/**
 * Data asset defining the visual appearance and equipment for a primitive tribal warrior character
 * Used by the Character Artist system to configure MetaHuman-based NPCs and player variants
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalWarriorAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorAsset();

    // Core character mesh and materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Mesh")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    // Equipment and accessories
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> SpearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> KnifeMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> BoneNecklaceMesh;

    // Character variant settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
    FLinearColor HairColor;

    // Behavior tags for AI system
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    TArray<FName> CharacterTags;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
    float SocialRank;

    // Asset validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool IsAssetValid() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    TArray<FString> GetMissingAssets() const;
};