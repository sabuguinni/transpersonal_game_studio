#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "CharacterCustomizationSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout")
};

UENUM(BlueprintType)
enum class EChar_ClothingType : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    PlantFiber      UMETA(DisplayName = "Plant Fiber"),
    Leather         UMETA(DisplayName = "Leather"),
    Fur             UMETA(DisplayName = "Fur"),
    BoneOrnaments   UMETA(DisplayName = "Bone Ornaments")
};

UENUM(BlueprintType)
enum class EChar_TribalMarking : uint8
{
    None        UMETA(DisplayName = "None"),
    Stripes     UMETA(DisplayName = "Stripes"),
    Dots        UMETA(DisplayName = "Dots"),
    Spirals     UMETA(DisplayName = "Spirals"),
    Geometric   UMETA(DisplayName = "Geometric"),
    Animal      UMETA(DisplayName = "Animal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    EChar_BodyType BodyType = EChar_BodyType::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float BodyScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float MuscleDefinition = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingType PrimaryClothing = EChar_ClothingType::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingType SecondaryClothing = EChar_ClothingType::PlantFiber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markings")
    EChar_TribalMarking FaceMarkings = EChar_TribalMarking::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Markings")
    EChar_TribalMarking BodyMarkings = EChar_TribalMarking::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    FChar_CustomizationData()
    {
        BodyType = EChar_BodyType::Hunter;
        BodyScale = 1.0f;
        MuscleDefinition = 0.8f;
        PrimaryClothing = EChar_ClothingType::AnimalHide;
        SecondaryClothing = EChar_ClothingType::PlantFiber;
        FaceMarkings = EChar_TribalMarking::None;
        BodyMarkings = EChar_TribalMarking::None;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterCustomizationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterCustomizationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Customization data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_CustomizationData CustomizationData;

    // Data table reference for presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TSoftObjectPtr<UDataTable> CustomizationPresets;

    // Character generation functions
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomization(const FChar_CustomizationData& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void GenerateRandomCharacter();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyClothing(EChar_ClothingType PrimaryClothing, EChar_ClothingType SecondaryClothing);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyTribalMarkings(EChar_TribalMarking FaceMarkings, EChar_TribalMarking BodyMarkings);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FChar_CustomizationData GetCurrentCustomization() const { return CustomizationData; }

    // Material application
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateCharacterMaterials();

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetSkinMaterial(UMaterialInterface* NewSkinMaterial);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetClothingMaterial(UMaterialInterface* NewClothingMaterial);

    UFUNCTION(BlueprintCallable, Category = "Materials")
    void SetHairMaterial(UMaterialInterface* NewHairMaterial);

private:
    // Helper functions
    void ApplyBodyScale();
    void ApplyMaterialsToMesh();
    USkeletalMeshComponent* GetCharacterMesh();

    // Cached references
    UPROPERTY()
    TObjectPtr<USkeletalMeshComponent> CachedMeshComponent;
};