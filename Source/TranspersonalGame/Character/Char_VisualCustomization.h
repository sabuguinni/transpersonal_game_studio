#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Char_VisualCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"), 
    Olive       UMETA(DisplayName = "Olive"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky")
};

UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None        UMETA(DisplayName = "None"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Scout       UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_BodyBuild BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EChar_TribalMarkings TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float ScarIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    float MuscleDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    bool bHasFacialHair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    FChar_VisualPreset()
    {
        PresetName = TEXT("Default");
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        TribalMarkings = EChar_TribalMarkings::None;
        ScarIntensity = 0.3f;
        MuscleDefinition = 0.5f;
        bHasFacialHair = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_VisualCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_VisualCustomization();

protected:
    virtual void BeginPlay() override;

    // Current visual configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual", meta = (AllowPrivateAccess = "true"))
    FChar_VisualPreset CurrentPreset;

    // Available presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    TArray<FChar_VisualPreset> AvailablePresets;

    // MetaHuman integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    bool bUseMetaHuman;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<TSoftObjectPtr<UMaterialInterface>> MetaHumanMaterials;

    // Character mesh reference
    UPROPERTY(BlueprintReadOnly, Category = "Character Visual")
    class USkeletalMeshComponent* CharacterMeshComponent;

public:
    // Visual customization methods
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void ApplyVisualPreset(const FChar_VisualPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetTribalMarkings(EChar_TribalMarkings NewMarkings);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetScarIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetMuscleDefinition(float Definition);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetFacialHair(bool bHasFacialHair);

    // MetaHuman methods
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void EnableMetaHuman(USkeletalMesh* MetaHumanMesh);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void DisableMetaHuman();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool IsUsingMetaHuman() const { return bUseMetaHuman; }

    // Preset management
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void LoadPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SaveCurrentAsPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    TArray<FString> GetAvailablePresetNames() const;

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character Visual")
    FChar_VisualPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintPure, Category = "Character Visual")
    EChar_SkinTone GetSkinTone() const { return CurrentPreset.SkinTone; }

    UFUNCTION(BlueprintPure, Category = "Character Visual")
    EChar_BodyBuild GetBodyBuild() const { return CurrentPreset.BodyBuild; }

    UFUNCTION(BlueprintPure, Category = "Character Visual")
    EChar_TribalMarkings GetTribalMarkings() const { return CurrentPreset.TribalMarkings; }

private:
    // Internal methods
    void InitializePresets();
    void UpdateCharacterMesh();
    void UpdateMaterials();
    void ApplyTribalMarkings();
    void ApplyBodyModifications();

    // Editor-only methods
#if WITH_EDITOR
public:
    UFUNCTION(CallInEditor)
    void RefreshCharacterVisuals();

    UFUNCTION(CallInEditor)
    void ResetToDefaultPreset();
#endif
};