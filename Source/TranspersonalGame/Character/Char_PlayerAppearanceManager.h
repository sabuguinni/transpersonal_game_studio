#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Char_PlayerAppearanceManager.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair        UMETA(DisplayName = "Fair"),
    Medium      UMETA(DisplayName = "Medium"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Tall        UMETA(DisplayName = "Tall")
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
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor EyeColor = FLinearColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    FChar_AppearanceData()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Athletic;
        Height = 1.0f;
        Weight = 1.0f;
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f); // Dark brown
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);   // Brown
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerAppearanceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerAppearanceManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Appearance")
    FChar_AppearanceData CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TArray<TSoftObjectPtr<USkeletalMesh>> AvailableBodyMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void ApplyAppearance(const FChar_AppearanceData& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    FChar_AppearanceData GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetBodyBuild(EChar_BodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Appearance")
    void SetClothing(USkeletalMesh* ClothingMesh, UMaterialInterface* ClothingMaterial);

protected:
    UFUNCTION()
    void UpdateCharacterMesh();

    UFUNCTION()
    void UpdateSkinMaterial();

    UFUNCTION()
    void UpdateClothing();

    UPROPERTY()
    class USkeletalMeshComponent* CharacterMesh;
};