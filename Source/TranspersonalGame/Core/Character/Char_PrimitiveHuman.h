#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "Char_PrimitiveHuman.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    ESurvivalGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FacialHairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> TribalMarkings;

    FChar_TribalAppearance()
    {
        CharacterName = TEXT("Tribal Human");
        Gender = ESurvivalGender::Male;
        SkinTone = 0.5f;
        HairColor = FLinearColor::Black;
        HairStyle = 0;
        FacialHairStyle = 0;
        BodyBuild = 0.7f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> PrimaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> SecondaryWeapon;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UStaticMesh> Shield;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<TSoftObjectPtr<UStaticMesh>> Jewelry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    FChar_TribalEquipment()
    {
        PrimaryWeapon = nullptr;
        SecondaryWeapon = nullptr;
        Shield = nullptr;
        ClothingMaterial = nullptr;
    }
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API AChar_PrimitiveHuman : public ACharacter
{
    GENERATED_BODY()

public:
    AChar_PrimitiveHuman();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Character Appearance System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_TribalEquipment Equipment;

    // Survival Stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Stamina;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Fear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Warmth;

    // Equipment Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* WeaponMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    UStaticMeshComponent* ShieldMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
    TArray<UStaticMeshComponent*> JewelryMeshes;

    // Character Functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void EquipTribalGear(const FChar_TribalEquipment& NewEquipment);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateSurvivalStats(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyEnvironmentalEffects(float Temperature, float Humidity, bool bIsNight);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetTribalMarkings(const TArray<FString>& Markings);

    UFUNCTION(BlueprintPure, Category = "Character")
    float GetOverallCondition() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool IsHealthy() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool NeedsFood() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool NeedsWater() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool IsExhausted() const;

    UFUNCTION(BlueprintPure, Category = "Character")
    bool IsAfraid() const;

private:
    void InitializeTribalCharacter();
    void CreateEquipmentComponents();
    void UpdateCharacterMaterials();
    void ProcessSurvivalDecay(float DeltaTime);

    UPROPERTY()
    float LastUpdateTime;

    UPROPERTY()
    bool bCharacterInitialized;
};