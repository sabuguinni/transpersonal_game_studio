#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_MetaHumanController.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair       UMETA(DisplayName = "Fair"),
    Medium     UMETA(DisplayName = "Medium"),
    Olive      UMETA(DisplayName = "Olive"),
    Dark       UMETA(DisplayName = "Dark"),
    VeryDark   UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean       UMETA(DisplayName = "Lean"),
    Athletic   UMETA(DisplayName = "Athletic"),
    Stocky     UMETA(DisplayName = "Stocky"),
    Muscular   UMETA(DisplayName = "Muscular")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor::Brown;

    FChar_AppearanceSettings()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        WeatheringLevel = 0.5f;
        ScarLevel = 0.3f;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor(0.4f, 0.2f, 0.1f, 1.0f); // Brown
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanController : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceSettings AppearanceSettings;

    // MetaHuman mesh references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Assets")
    TObjectPtr<USkeletalMesh> BaseFemaleMesh;

    // Material instances for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> HairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TObjectPtr<UMaterialInterface> EyeMaterial;

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearanceSettings();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWeatheringLevel(float NewWeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScarLevel(float NewScarLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetEyeColor(FLinearColor NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void LoadPresetAppearance(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SaveAppearancePreset(const FString& PresetName);

private:
    // Internal functions
    void UpdateSkinMaterial();
    void UpdateHairMaterial();
    void UpdateEyeMaterial();
    void UpdateMeshMorphTargets();

    // Component references
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> TargetMeshComponent;

    // Cached material instances
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicHairMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicEyeMaterial;
};