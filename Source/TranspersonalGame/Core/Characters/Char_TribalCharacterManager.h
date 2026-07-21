#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyFat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    FChar_TribalAppearance()
    {
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        MuscleMass = 0.7f;
        BodyFat = 0.15f;
        Height = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> TorsoClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> LegClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UStaticMesh> FootClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<TSoftObjectPtr<UStaticMesh>> Jewelry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor ClothingColor;

    FChar_TribalClothing()
    {
        ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
    }
};

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Shaman      UMETA(DisplayName = "Shaman")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    UDataTable* TribalAppearanceTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    UDataTable* TribalClothingTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<TSoftObjectPtr<USkeletalMesh>> MaleBaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<TSoftObjectPtr<USkeletalMesh>> FemaleBaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<TSoftObjectPtr<UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    int32 MaxCharactersToGenerate;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> GeneratedCharacters;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateTribalCharacter(const FTransform& SpawnTransform, bool bIsMale = true, EChar_TribalRole Role = EChar_TribalRole::Hunter);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void GenerateRandomTribalGroup(int32 GroupSize, const FVector& CenterLocation, float SpreadRadius);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalAppearance GenerateRandomAppearance(bool bIsMale, EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalClothing GenerateRandomClothing(EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyAppearanceToCharacter(AActor* Character, const FChar_TribalAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyClothingToCharacter(AActor* Character, const FChar_TribalClothing& Clothing);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void GenerateTestCharacters();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void ClearGeneratedCharacters();

private:
    FLinearColor GetSkinToneForRole(EChar_TribalRole Role);
    float GetMuscleMassForRole(EChar_TribalRole Role, bool bIsMale);
    FLinearColor GetClothingColorForRole(EChar_TribalRole Role);
};