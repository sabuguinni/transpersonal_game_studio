#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Char_MetaHumanSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Scout       UMETA(DisplayName = "Scout"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Healer      UMETA(DisplayName = "Healer")
};

UENUM(BlueprintType)
enum class EChar_PhysicalBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall"),
    Short       UMETA(DisplayName = "Short")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    EChar_PhysicalBuild PhysicalBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> EquipmentList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backstory")
    FString BackgroundStory;

    FChar_TribalCharacterData()
    {
        CharacterName = TEXT("Unnamed");
        TribalRole = EChar_TribalRole::Hunter;
        PhysicalBuild = EChar_PhysicalBuild::Athletic;
        Height = 170.0f;
        Weight = 70.0f;
        SkinTone = TEXT("Medium");
        HairColor = TEXT("Dark Brown");
        EyeColor = TEXT("Brown");
        ClothingStyle = TEXT("Animal Hide");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FChar_TribalCharacterData CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TMap<FString, float> MorphTargetValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<class UMaterialInterface*> ClothingMaterials;

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void ApplyTribalCharacterData(const FChar_TribalCharacterData& NewCharacterData);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void RandomizeTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyMorphTargets();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinMaterial(int32 MaterialIndex);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetClothingMaterial(int32 MaterialIndex);

    UFUNCTION(BlueprintCallable, Category = "Equipment")
    void AttachTribalEquipment();

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCharacterSetup() const;

private:
    void InitializeDefaultMorphTargets();
    void SetupTribalClothing();
    void ApplyWeatheringEffects();
    FChar_TribalCharacterData GenerateRandomTribalData();
};

#include "Char_MetaHumanSystem.generated.h"