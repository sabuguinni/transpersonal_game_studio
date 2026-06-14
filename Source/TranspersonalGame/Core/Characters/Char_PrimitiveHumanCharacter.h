#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/Engine.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    bool bWearingAnimalHide = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    bool bWearingBoneAccessories = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    FLinearColor HideColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    float WearLevel = 0.5f; // 0.0 = new, 1.0 = heavily worn

    FChar_TribalClothing()
    {
        bWearingAnimalHide = true;
        bWearingBoneAccessories = true;
        HideColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        WearLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PrimitiveWeapons
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    bool bHasStoneSpear = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    bool bHasStoneAxe = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    bool bHasBoneKnife = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapons")
    float WeaponDurability = 1.0f;

    FChar_PrimitiveWeapons()
    {
        bHasStoneSpear = true;
        bHasStoneAxe = false;
        bHasBoneKnife = false;
        WeaponDurability = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_TribalClothing TribalClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FChar_PrimitiveWeapons PrimitiveWeapons;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    float SkinWeatheringLevel = 0.6f; // How weathered the skin appears

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    // Weapon components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    UStaticMeshComponent* SpearMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    UStaticMeshComponent* AxeMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapons")
    UStaticMeshComponent* KnifeMeshComponent;

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalClothing(const FChar_TribalClothing& NewClothing);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetPrimitiveWeapons(const FChar_PrimitiveWeapons& NewWeapons);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinWeathering(float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(FLinearColor NewSkinTone);

    // Weapon management
    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void EquipSpear();

    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void EquipAxe();

    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void EquipKnife();

    UFUNCTION(BlueprintCallable, Category = "Weapons")
    void UnequipAllWeapons();

    // Character state queries
    UFUNCTION(BlueprintPure, Category = "Character State")
    bool IsWearingTribalClothing() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    bool HasAnyWeapon() const;

    UFUNCTION(BlueprintPure, Category = "Character State")
    float GetOverallCondition() const;

private:
    void UpdateCharacterAppearance();
    void UpdateWeaponVisibility();
    void CreateWeaponComponents();
};