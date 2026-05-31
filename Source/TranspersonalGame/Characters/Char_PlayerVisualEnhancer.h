#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "Char_PlayerVisualEnhancer.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> ChestPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> LegPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> FootPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> HeadPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    FChar_TribalClothingSet()
    {
        ChestPiece = nullptr;
        LegPiece = nullptr;
        FootPiece = nullptr;
        HeadPiece = nullptr;
        ClothingMaterial = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAccessorySet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TSoftObjectPtr<UStaticMesh> BoneNecklace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TSoftObjectPtr<UStaticMesh> FeatherHeadband;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TSoftObjectPtr<UStaticMesh> StoneArmband;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TSoftObjectPtr<UStaticMesh> ClawEarring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Accessories")
    TSoftObjectPtr<UMaterialInterface> AccessoryMaterial;

    FChar_TribalAccessorySet()
    {
        BoneNecklace = nullptr;
        FeatherHeadband = nullptr;
        StoneArmband = nullptr;
        ClawEarring = nullptr;
        AccessoryMaterial = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PlayerVisualEnhancer : public AActor
{
    GENERATED_BODY()

public:
    AChar_PlayerVisualEnhancer();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TArray<FChar_TribalClothingSet> TribalClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TArray<FChar_TribalAccessorySet> TribalAccessorySets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TArray<TSoftObjectPtr<USkeletalMesh>> TribalCharacterMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    TArray<TSoftObjectPtr<UMaterialInterface>> HairMaterials;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void EnhancePlayerCharacter(class ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void ApplyTribalClothing(class ATranspersonalCharacter* Character, int32 ClothingSetIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void ApplyTribalAccessories(class ATranspersonalCharacter* Character, int32 AccessorySetIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void ApplyRandomTribalLook(class ATranspersonalCharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void SetCharacterMesh(class ATranspersonalCharacter* Character, int32 MeshIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void ApplySkinMaterial(class ATranspersonalCharacter* Character, int32 MaterialIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void ApplyHairMaterial(class ATranspersonalCharacter* Character, int32 MaterialIndex);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void AddWeatheringEffects(class ATranspersonalCharacter* Character, float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Character Enhancement")
    void AddBattleScars(class ATranspersonalCharacter* Character, int32 ScarCount);

private:
    void InitializeTribalClothingSets();
    void InitializeTribalAccessorySets();
    void InitializeCharacterMeshes();
    void InitializeMaterials();

    UStaticMeshComponent* CreateAccessoryComponent(const FString& ComponentName);
    void AttachAccessoryToSocket(UStaticMeshComponent* AccessoryComponent, USkeletalMeshComponent* TargetMesh, const FName& SocketName);
};