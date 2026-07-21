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
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FacialMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 Age;

    FChar_TribalAppearance()
    {
        CharacterName = TEXT("Tribal Member");
        SkinTone = 0;
        HairStyle = 0;
        FacialMarkings = 0;
        Height = 175.0f;
        Weight = 70.0f;
        Age = 25;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalRole
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    FString RoleName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    TArray<FString> EquipmentList;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    float StrengthModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    float AgilityModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Role")
    float IntelligenceModifier;

    FChar_TribalRole()
    {
        RoleName = TEXT("Hunter");
        EquipmentList = {TEXT("Stone Spear"), TEXT("Hide Clothing")};
        StrengthModifier = 1.0f;
        AgilityModifier = 1.0f;
        IntelligenceModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<USkeletalMesh*> BaseMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<UStaticMesh*> HairMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<UStaticMesh*> ClothingMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    TArray<UStaticMesh*> WeaponMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    UDataTable* TribalRolesTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Database")
    UDataTable* AppearanceVariationsTable;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalAppearance GenerateRandomAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FChar_TribalRole GetTribalRole(const FString& RoleName);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    class APawn* SpawnTribalCharacter(const FVector& Location, const FRotator& Rotation, const FChar_TribalAppearance& Appearance, const FChar_TribalRole& Role);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void ApplyAppearanceToCharacter(class USkeletalMeshComponent* MeshComponent, const FChar_TribalAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    void EquipCharacterWithRole(class APawn* Character, const FChar_TribalRole& Role);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    TArray<FString> GetAvailableRoles();

    UFUNCTION(BlueprintCallable, Category = "Diversity")
    void GenerateTribalCommunity(const FVector& CenterLocation, int32 PopulationSize, float SpawnRadius);

private:
    int32 GetRandomSkinTone();
    int32 GetRandomHairStyle();
    int32 GetRandomFacialMarkings();
    float GetRandomHeight();
    float GetRandomWeight();
    int32 GetRandomAge();
};