#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialParameterCollection.h"
#include "Char_VisualCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    VeryDark    UMETA(DisplayName = "Very Dark")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean Survivor"),
    Athletic    UMETA(DisplayName = "Athletic Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Weathered   UMETA(DisplayName = "Weathered Elder")
};

UENUM(BlueprintType)
enum class EChar_TribalMarkings : uint8
{
    None        UMETA(DisplayName = "No Markings"),
    Hunter      UMETA(DisplayName = "Hunter Marks"),
    Warrior     UMETA(DisplayName = "Warrior Scars"),
    Shaman      UMETA(DisplayName = "Ritual Marks"),
    Elder       UMETA(DisplayName = "Wisdom Lines"),
    Survivor    UMETA(DisplayName = "Survival Scars")
};

UENUM(BlueprintType)
enum class EChar_HairStyle : uint8
{
    Long        UMETA(DisplayName = "Long Wild"),
    Braided     UMETA(DisplayName = "Braided"),
    Short       UMETA(DisplayName = "Short Rough"),
    Shaved      UMETA(DisplayName = "Partially Shaved"),
    Matted      UMETA(DisplayName = "Matted"),
    Decorated   UMETA(DisplayName = "Bone Decorated")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Garb"),
    Gatherer    UMETA(DisplayName = "Gatherer Wraps"),
    Warrior     UMETA(DisplayName = "Warrior Armor"),
    Shaman      UMETA(DisplayName = "Ritual Robes"),
    Elder       UMETA(DisplayName = "Elder Furs")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarkings TribalMarkings = EChar_TribalMarkings::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_HairStyle HairStyle = EChar_HairStyle::Long;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float BodyScale = 1.0f;

    FChar_AppearanceData()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        TribalMarkings = EChar_TribalMarkings::None;
        HairStyle = EChar_HairStyle::Long;
        ClothingStyle = EChar_ClothingStyle::Hunter;
        HairColor = FLinearColor::Black;
        EyeColor = FLinearColor::Brown;
        WeatheringLevel = 0.5f;
        ScarLevel = 0.3f;
        BodyScale = 1.0f;
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

    // Current appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData CurrentAppearance;

    // Material references for customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseHairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseClothingMaterial;

    // MetaHuman integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class USkeletalMesh* MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<class UMaterialInterface*> SkinToneMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<class UMaterialInterface*> HairStyleMaterials;

    // Customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetTribalMarkings(EChar_TribalMarkings NewMarkings);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetHairStyle(EChar_HairStyle NewHairStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetClothingStyle(EChar_ClothingStyle NewClothingStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetWeatheringLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetScarLevel(float NewLevel);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    FChar_AppearanceData GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SaveAppearanceToFile(const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    bool LoadAppearanceFromFile(const FString& FileName);

private:
    // Internal helper functions
    void ApplySkinToneMaterial();
    void ApplyBodyBuildScale();
    void ApplyTribalMarkingsTexture();
    void ApplyHairStyleMesh();
    void ApplyClothingMesh();
    void UpdateMaterialParameters();

    // Component references
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    class UMaterialParameterCollection* AppearanceParameters;
};