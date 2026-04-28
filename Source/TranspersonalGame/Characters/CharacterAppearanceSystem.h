#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "../SharedTypes.h"
#include "CharacterAppearanceSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child"),
    Gatherer    UMETA(DisplayName = "Gatherer")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Bronze      UMETA(DisplayName = "Bronze"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Elderly     UMETA(DisplayName = "Elderly"),
    Child       UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float AgeModifier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTattoos = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasPaintMarkings = false;

    FChar_AppearanceData()
    {
        TribalRole = EChar_TribalRole::Hunter;
        SkinTone = EChar_SkinTone::Medium;
        BodyType = EChar_BodyType::Athletic;
        SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        AgeModifier = 0.5f;
        WeatheringAmount = 0.3f;
        bHasTattoos = false;
        bHasScars = false;
        bHasPaintMarkings = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearAmount = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasFur = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasLeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bHasBoneDecorations = false;

    FChar_ClothingData()
    {
        PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        SecondaryColor = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f);
        WearAmount = 0.2f;
        bHasFur = false;
        bHasLeather = true;
        bHasBoneDecorations = false;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core appearance data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData AppearanceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_ClothingData ClothingData;

    // Material instances for customization
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    TObjectPtr<UMaterialInstanceDynamic> SkinMaterialInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    TObjectPtr<UMaterialInstanceDynamic> HairMaterialInstance;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    TObjectPtr<UMaterialInstanceDynamic> ClothingMaterialInstance;

    // Skeletal mesh components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    TObjectPtr<USkeletalMeshComponent> CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Appearance")
    TObjectPtr<USkeletalMeshComponent> ClothingMesh;

    // Appearance modification functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearanceData(const FChar_AppearanceData& NewAppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyClothingData(const FChar_ClothingData& NewClothingData);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void AddTattoos(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void AddScars(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void AddPaintMarkings(bool bEnable);

    // Randomization functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeForRole(EChar_TribalRole Role);

    // Preset functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetHunterPreset();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetElderPreset();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetScoutPreset();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetShamanPreset();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetChildPreset();

private:
    void InitializeMaterialInstances();
    void UpdateSkinMaterial();
    void UpdateHairMaterial();
    void UpdateClothingMaterial();
    void SetupMeshComponents();
    FLinearColor GetSkinColorForTone(EChar_SkinTone SkinTone);
    float GetBodyScaleForType(EChar_BodyType BodyType);
};