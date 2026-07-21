#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

USTRUCT(BlueprintType)
struct FChar_CharacterCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    ESkinTone SkinTone = ESkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EHairColor HairColor = EHairColor::Brown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EBodyBuild BodyBuild = EBodyBuild::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bWearingAnimalHide = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    bool bWearingBoneJewelry = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
    bool bCarryingStoneAxe = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
    bool bCarryingSpear = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFacePaint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float WeatheringLevel = 0.5f;
};

USTRUCT(BlueprintType)
struct FChar_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float MaxStamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Temperature = 37.0f; // Body temperature in Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f; // Fear level from 0-100
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_PrimitiveHumanCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHumanCharacter();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character Customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character", meta = (AllowPrivateAccess = "true"))
    FChar_CharacterCustomization CharacterCustomization;

    // Survival Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (AllowPrivateAccess = "true"))
    FChar_SurvivalStats SurvivalStats;

    // Mesh Components for Clothing and Tools
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* AnimalHideMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* BoneJewelryMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* StoneAxeMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
    UStaticMeshComponent* SpearMesh;

    // Material Instances for Customization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* HairMaterial;

public:
    // Character Customization Functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyCharacterCustomization(const FChar_CharacterCustomization& NewCustomization);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetSkinTone(ESkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetHairColor(EHairColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetBodyBuild(EBodyBuild NewBodyBuild);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ToggleAnimalHide(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ToggleBoneJewelry(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ToggleStoneAxe(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ToggleSpear(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetFacePaint(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetWeatheringLevel(float Level);

    // Survival Functions
    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void TakeDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void RestoreHealth(float HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ConsumeFood(float FoodValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void DrinkWater(float WaterValue);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UseStamina(float StaminaCost);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void AddFear(float FearAmount);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void ReduceFear(float FearReduction);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Character")
    FChar_CharacterCustomization GetCharacterCustomization() const { return CharacterCustomization; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    FChar_SurvivalStats GetSurvivalStats() const { return SurvivalStats; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetHealthPercentage() const { return SurvivalStats.Health / SurvivalStats.MaxHealth; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    float GetStaminaPercentage() const { return SurvivalStats.Stamina / SurvivalStats.MaxStamina; }

    UFUNCTION(BlueprintPure, Category = "Survival")
    bool IsAlive() const { return SurvivalStats.Health > 0.0f; }

private:
    void UpdateMaterialParameters();
    void UpdateMeshVisibility();
    void ProcessSurvivalDecay(float DeltaTime);
};