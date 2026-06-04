#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_VisualCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Tall        UMETA(DisplayName = "Tall Scout"),
    Elder       UMETA(DisplayName = "Weathered Elder")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Tanned      UMETA(DisplayName = "Sun-Tanned"),
    Weathered   UMETA(DisplayName = "Weather-Beaten")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    PlantFiber      UMETA(DisplayName = "Plant Fiber"),
    BoneOrnaments   UMETA(DisplayName = "Bone Ornaments"),
    FeatheredGarb   UMETA(DisplayName = "Feathered Garb"),
    MinimalWrap     UMETA(DisplayName = "Minimal Wrap")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Lean;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasWarPaint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleDefinition = 0.5f;

    FChar_AppearanceData()
    {
        BodyType = EChar_BodyType::Lean;
        SkinTone = EChar_SkinTone::Medium;
        ClothingStyle = EChar_ClothingStyle::AnimalHide;
        SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        bHasWarPaint = false;
        bHasScars = false;
        MuscleDefinition = 0.5f;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Assets")
    TArray<TSoftObjectPtr<USkeletalMesh>> BodyTypeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Assets")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWarPaint(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetScars(bool bEnabled);

protected:
    UFUNCTION()
    void UpdateMeshBasedOnBodyType();

    UFUNCTION()
    void UpdateMaterialsBasedOnAppearance();

    UFUNCTION()
    USkeletalMeshComponent* GetCharacterMesh() const;

    UFUNCTION()
    void ApplyMaterialParameters();

private:
    bool bAppearanceApplied = false;
    float LastUpdateTime = 0.0f;
};