#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterAppearanceSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    Deep        UMETA(DisplayName = "Deep")
};

UENUM(BlueprintType)
enum class EChar_ClothingSet : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Gear"),
    Gatherer    UMETA(DisplayName = "Gatherer Outfit"),
    Shaman      UMETA(DisplayName = "Tribal Shaman"),
    Warrior     UMETA(DisplayName = "Warrior Armor")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_AppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ClothingSet ClothingSet = EChar_ClothingSet::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FVector BodyScale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    FChar_AppearanceData()
    {
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        ClothingSet = EChar_ClothingSet::Hunter;
        BodyScale = FVector(1.0f, 1.0f, 1.0f);
        SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_AppearanceData CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TObjectPtr<USkeletalMesh> BaseFemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TArray<TObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    TArray<TObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void SetClothingSet(EChar_ClothingSet NewClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Character Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintPure, Category = "Character Appearance")
    FChar_AppearanceData GetCurrentAppearance() const { return CurrentAppearance; }

private:
    void UpdateMeshScale();
    void UpdateSkinMaterial();
    void UpdateClothingMaterial();
    
    UPROPERTY()
    TObjectPtr<class USkeletalMeshComponent> CachedMeshComponent;
};