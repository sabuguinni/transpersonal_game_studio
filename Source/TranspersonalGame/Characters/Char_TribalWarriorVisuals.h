#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "Char_TribalWarriorVisuals.generated.h"

UENUM(BlueprintType)
enum class EChar_WarriorArmorType : uint8
{
    None = 0,
    BoneArmor = 1,
    LeatherArmor = 2,
    HideArmor = 3,
    ScaleArmor = 4,
    CompositeArmor = 5
};

UENUM(BlueprintType)
enum class EChar_WeaponType : uint8
{
    None = 0,
    StoneSpear = 1,
    BoneSpear = 2,
    WoodenClub = 3,
    StoneAxe = 4,
    BoneKnife = 5,
    Sling = 6
};

UENUM(BlueprintType)
enum class EChar_ScarPattern : uint8
{
    None = 0,
    ClawMarks = 1,
    BiteMarks = 2,
    BurnScars = 3,
    CombatScars = 4,
    RitualScars = 5
};

USTRUCT(BlueprintType)
struct FChar_WarriorCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_WarriorArmorType ArmorType = EChar_WarriorArmorType::BoneArmor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_WeaponType PrimaryWeapon = EChar_WeaponType::StoneSpear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_ScarPattern ScarType = EChar_ScarPattern::ClawMarks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleDefinition = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BattleWear = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacialHair = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneOrnaments = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalWarriorVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalWarriorVisuals();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Visuals")
    FChar_WarriorCustomization CurrentCustomization;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Assets")
    TSoftObjectPtr<USkeletalMesh> BaseMaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Assets")
    TSoftObjectPtr<USkeletalMesh> BaseFemaleMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> ArmorMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> WeaponMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TArray<TSoftObjectPtr<UTexture2D>> ScarTextures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TArray<TSoftObjectPtr<UTexture2D>> TattooTextures;

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplyCustomization(const FChar_WarriorCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetArmorType(EChar_WarriorArmorType NewArmorType);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetWeapon(EChar_WeaponType NewWeapon);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void AddBattleDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void ApplySkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Visuals")
    void SetMuscleDefinition(float MuscleLevel);

protected:
    UFUNCTION()
    void UpdateMeshMaterials();

    UFUNCTION()
    void ApplyArmorVariant();

    UFUNCTION()
    void AttachWeaponMesh();

    UFUNCTION()
    void ApplyScarPattern();

    UPROPERTY()
    class USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

private:
    void InitializeMaterialInstances();
    void LoadAssetReferences();
    FLinearColor GenerateRandomSkinTone();
    EChar_ScarPattern SelectRandomScarPattern();
};