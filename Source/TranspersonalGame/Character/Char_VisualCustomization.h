#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_VisualCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Olive       UMETA(DisplayName = "Olive"),
    Brown       UMETA(DisplayName = "Brown"),
    Dark        UMETA(DisplayName = "Dark")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Hunter"),
    Athletic    UMETA(DisplayName = "Athletic Survivor"),
    Stocky      UMETA(DisplayName = "Stocky Gatherer"),
    Muscular    UMETA(DisplayName = "Muscular Warrior")
};

UENUM(BlueprintType)
enum class EChar_ClothingStyle : uint8
{
    Primitive   UMETA(DisplayName = "Animal Hide Basic"),
    Hunter      UMETA(DisplayName = "Hunter Leathers"),
    Gatherer    UMETA(DisplayName = "Woven Plant Fibers"),
    Tribal      UMETA(DisplayName = "Decorated Tribal Wear")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    EChar_ClothingStyle ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scars & Marks")
    bool bHasFacialScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scars & Marks")
    bool bHasBodyScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weathering")
    float SkinWeathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dirt & Grime")
    float DirtLevel;

    FChar_VisualProfile()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyType = EChar_BodyType::Athletic;
        ClothingStyle = EChar_ClothingStyle::Primitive;
        Height = 1.75f;
        Weight = 70.0f;
        bHasFacialScars = false;
        bHasBodyScars = false;
        SkinWeathering = 0.3f;
        DirtLevel = 0.5f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_VisualProfile VisualProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Assets")
    TObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Assets")
    TObjectPtr<USkeletalMesh> BaseFemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyVisualProfile(const FChar_VisualProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetClothingStyle(EChar_ClothingStyle NewStyle);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void AddScar(bool bFacialScar = true);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetWeatheringLevel(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetDirtLevel(float DirtAmount);

    UFUNCTION(BlueprintPure, Category = "Character Customization")
    FChar_VisualProfile GetCurrentVisualProfile() const { return VisualProfile; }

private:
    void UpdateCharacterMesh();
    void UpdateMaterials();
    USkeletalMeshComponent* GetOwnerMeshComponent() const;
};