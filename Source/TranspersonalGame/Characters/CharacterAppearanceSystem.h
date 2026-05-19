#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "CharacterAppearanceSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout")
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

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall"),
    Average     UMETA(DisplayName = "Average")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearancePreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    FChar_AppearancePreset()
    {
        PresetName = TEXT("Default");
        TribalRole = EChar_TribalRole::Hunter;
        SkinTone = EChar_SkinTone::Medium;
        BodyType = EChar_BodyType::Average;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationOptions
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float MuscleDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float SkinWeathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    float ScarsIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor SkinBaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    bool bHasTribalPaint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FLinearColor TribalPaintColor;

    FChar_CustomizationOptions()
    {
        MuscleDefinition = 0.5f;
        SkinWeathering = 0.3f;
        ScarsIntensity = 0.2f;
        SkinBaseColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        bHasTribalPaint = false;
        TribalPaintColor = FLinearColor::Red;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAppearanceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAppearanceSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance System")
    TArray<FChar_AppearancePreset> TribalPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance System")
    FChar_CustomizationOptions CurrentCustomization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance System")
    EChar_TribalRole CurrentRole;

    UPROPERTY(BlueprintReadOnly, Category = "Appearance System", meta = (AllowPrivateAccess = "true"))
    USkeletalMeshComponent* TargetMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<USkeletalMesh*> BaseMeshVariants;

public:
    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void InitializeAppearanceSystem(USkeletalMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void ApplyTribalRole(EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void ApplyAppearancePreset(const FChar_AppearancePreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void ApplyCustomization(const FChar_CustomizationOptions& Options);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void SetSkinTone(EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    void SetBodyType(EChar_BodyType BodyType);

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    FChar_AppearancePreset GetCurrentPreset() const;

    UFUNCTION(BlueprintCallable, Category = "Appearance System")
    TArray<FChar_AppearancePreset> GetAvailablePresets() const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void GenerateDefaultPresets();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Development")
    void ValidateAppearanceAssets();

private:
    void CreateTribalPresets();
    void ApplyMaterialParameters(UMaterialInstanceDynamic* MaterialInstance, const FChar_CustomizationOptions& Options);
    UMaterialInstanceDynamic* CreateDynamicMaterial(UMaterialInterface* BaseMaterial);
    void UpdateMeshMorphTargets(const FChar_CustomizationOptions& Options);
};