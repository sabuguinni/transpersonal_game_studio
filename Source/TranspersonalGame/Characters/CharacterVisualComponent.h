#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "CharacterVisualComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean Scout"),
    Athletic    UMETA(DisplayName = "Athletic Hunter"),
    Muscular    UMETA(DisplayName = "Muscular Warrior"),
    Heavy       UMETA(DisplayName = "Heavy Gatherer")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Olive       UMETA(DisplayName = "Olive"),
    Bronze      UMETA(DisplayName = "Bronze"),
    Dark        UMETA(DisplayName = "Dark")
};

UENUM(BlueprintType)
enum class EChar_ClothingSet : uint8
{
    Minimal     UMETA(DisplayName = "Minimal Hide"),
    Hunter      UMETA(DisplayName = "Hunter Leather"),
    Gatherer    UMETA(DisplayName = "Gatherer Fiber"),
    Warrior     UMETA(DisplayName = "Warrior Bone")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_VisualSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    EChar_ClothingSet ClothingSet = EChar_ClothingSet::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    FString FacePaintPattern = "None";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    FString HairStyle = "Long";

    FChar_VisualSettings()
    {
        BodyType = EChar_BodyType::Athletic;
        SkinTone = EChar_SkinTone::Medium;
        ClothingSet = EChar_ClothingSet::Hunter;
        FacePaintPattern = "None";
        HairStyle = "Long";
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterVisualComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterVisualComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character visual settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visual")
    FChar_VisualSettings VisualSettings;

    // Mesh references for different body types
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Meshes")
    TObjectPtr<USkeletalMesh> LeanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Meshes")
    TObjectPtr<USkeletalMesh> AthleticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Meshes")
    TObjectPtr<USkeletalMesh> MuscularMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Meshes")
    TObjectPtr<USkeletalMesh> HeavyMesh;

    // Material references for skin tones
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Materials")
    TArray<TObjectPtr<UMaterialInterface>> SkinToneMaterials;

    // Material references for clothing sets
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Visual|Materials")
    TArray<TObjectPtr<UMaterialInterface>> ClothingMaterials;

    // Apply visual settings to character
    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void ApplyVisualSettings();

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void SetClothingSet(EChar_ClothingSet NewClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Character Visual")
    void RandomizeAppearance();

private:
    // Helper functions
    USkeletalMesh* GetMeshForBodyType(EChar_BodyType BodyType) const;
    UMaterialInterface* GetSkinMaterial(EChar_SkinTone SkinTone) const;
    UMaterialInterface* GetClothingMaterial(EChar_ClothingSet ClothingSet) const;
    
    // Cache references
    UPROPERTY()
    TObjectPtr<class USkeletalMeshComponent> CachedMeshComponent;
};

#include "CharacterVisualComponent.generated.h"