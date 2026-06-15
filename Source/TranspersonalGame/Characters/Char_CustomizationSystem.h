#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Char_CustomizationSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_ClothingType : uint8
{
    AnimalHide_Basic     UMETA(DisplayName = "Basic Animal Hide"),
    AnimalHide_Decorated UMETA(DisplayName = "Decorated Animal Hide"),
    PlantFiber_Woven     UMETA(DisplayName = "Woven Plant Fiber"),
    Leather_Crafted      UMETA(DisplayName = "Crafted Leather"),
    Fur_Winter           UMETA(DisplayName = "Winter Fur")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean_Hunter          UMETA(DisplayName = "Lean Hunter"),
    Strong_Warrior       UMETA(DisplayName = "Strong Warrior"),
    Wise_Elder           UMETA(DisplayName = "Wise Elder"),
    Agile_Scout          UMETA(DisplayName = "Agile Scout"),
    Sturdy_Gatherer      UMETA(DisplayName = "Sturdy Gatherer")
};

UENUM(BlueprintType)
enum class EChar_TribalMarking : uint8
{
    None                 UMETA(DisplayName = "No Markings"),
    Hunter_Stripes       UMETA(DisplayName = "Hunter Stripes"),
    Warrior_Scars        UMETA(DisplayName = "Warrior Scars"),
    Elder_Symbols        UMETA(DisplayName = "Elder Symbols"),
    Scout_Dots           UMETA(DisplayName = "Scout Dots")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CustomizationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingType ClothingType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalMarking TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> EquippedTools;

    FChar_CustomizationData()
    {
        ClothingType = EChar_ClothingType::AnimalHide_Basic;
        BodyType = EChar_BodyType::Lean_Hunter;
        TribalMarkings = EChar_TribalMarking::None;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_CustomizationSystem : public AActor
{
    GENERATED_BODY()

public:
    AChar_CustomizationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DisplayMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_CustomizationData DefaultCustomization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EChar_ClothingType, UStaticMesh*> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EChar_BodyType, USkeletalMesh*> BodyMeshes;

public:
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomization(const FChar_CustomizationData& CustomizationData);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    FChar_CustomizationData GetRandomCustomization();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetClothingType(EChar_ClothingType NewClothingType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetTribalMarkings(EChar_TribalMarking NewMarkings);

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateCustomizationData(const FChar_CustomizationData& Data);

private:
    void UpdateDisplayMesh();
    void LoadDefaultAssets();
};

#include "Char_CustomizationSystem.generated.h"