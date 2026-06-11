#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Char_MetaHumanManager.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Average     UMETA(DisplayName = "Average Build"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Elderly     UMETA(DisplayName = "Elderly Wise")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light Tan"),
    Medium      UMETA(DisplayName = "Medium Brown"),
    Dark        UMETA(DisplayName = "Dark Brown"),
    Weathered   UMETA(DisplayName = "Sun Weathered"),
    Pale        UMETA(DisplayName = "Cave Pale")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    AnimalHide      UMETA(DisplayName = "Animal Hide"),
    WovenGrass      UMETA(DisplayName = "Woven Grass"),
    LeatherWraps    UMETA(DisplayName = "Leather Wraps"),
    BoneArmor       UMETA(DisplayName = "Bone Armor"),
    Minimal         UMETA(DisplayName = "Minimal Covering")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EChar_BodyType BodyType = EChar_BodyType::Average;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingStyle ClothingStyle = EChar_ClothingStyle::AnimalHide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalPaint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName = TEXT("Unnamed");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString TribeName = TEXT("Wanderer");

    FChar_CharacterTraits()
    {
        BodyType = EChar_BodyType::Average;
        SkinTone = EChar_SkinTone::Medium;
        Height = 1.0f;
        Weight = 1.0f;
        ClothingStyle = EChar_ClothingStyle::AnimalHide;
        bHasTribalPaint = false;
        bHasBoneJewelry = false;
        bHasScars = false;
        CharacterName = TEXT("Unnamed");
        TribeName = TEXT("Wanderer");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // CHARACTER CUSTOMIZATION
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyCharacterTraits(const FChar_CharacterTraits& Traits);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    FChar_CharacterTraits GenerateRandomTraits();

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

    // METAHUMAN INTEGRATION
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void LoadMetaHumanPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SaveCurrentConfiguration(const FString& ConfigName);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    TArray<FString> GetAvailablePresets();

    // CHARACTER DIVERSITY
    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void CreateCharacterVariant(const FChar_CharacterTraits& BaseTraits, int32 VariationSeed);

    UFUNCTION(BlueprintCallable, Category = "Character Diversity")
    void PopulateNPCsWithDiversity(int32 NPCCount, float DiversityFactor);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FChar_CharacterTraits CurrentTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FChar_CharacterTraits> PresetTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<class USkeletalMesh*> ClothingMeshes;

private:
    void InitializeDefaultTraits();
    void ApplyPhysicalTraits(const FChar_CharacterTraits& Traits);
    void ApplyClothingAndAccessories(const FChar_CharacterTraits& Traits);
    void UpdateMaterialParameters(const FChar_CharacterTraits& Traits);
};