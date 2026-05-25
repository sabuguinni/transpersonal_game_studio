#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/SkeletalMeshComponent.h"
#include "../SharedTypes.h"
#include "TribalCharacterComponent.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Dark        UMETA(DisplayName = "Dark"),
    Bronze      UMETA(DisplayName = "Bronze"),
    Weathered   UMETA(DisplayName = "Weathered")
};

UENUM(BlueprintType)
enum class EChar_BodyBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Elderly     UMETA(DisplayName = "Elderly"),
    Child       UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyBuild BodyBuild = EChar_BodyBuild::Muscular;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalScars = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasBoneJewelry = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasFeathers = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float MuscleMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 25.0f;

    FChar_TribalAppearance()
    {
        SkinTone = EChar_SkinTone::Medium;
        BodyBuild = EChar_BodyBuild::Muscular;
        SkinColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        bHasTribalScars = true;
        bHasBoneJewelry = true;
        bHasFeathers = false;
        MuscleMass = 1.0f;
        Age = 25.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalCharacterComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalCharacterComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance and role
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalRole TribalRole = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Tribal Hunter");

    // Character stats
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Strength = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Agility = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float Wisdom = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float SurvivalSkill = 80.0f;

    // Equipment and tools
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    TArray<FString> EquippedTools;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString PrimaryWeapon = TEXT("Stone Spear");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    FString ClothingStyle = TEXT("Hide Wrap");

    // Character customization functions
    UFUNCTION(BlueprintCallable, Category = "Character")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyAppearanceToMesh(USkeletalMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void UpdateCharacterStats();

    UFUNCTION(BlueprintCallable, Category = "Character")
    FString GetRoleDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Character")
    TArray<FString> GetAppropriateTools() const;

private:
    // Internal appearance management
    void ApplySkinTone(USkeletalMeshComponent* MeshComponent);
    void ApplyBodyModifications(USkeletalMeshComponent* MeshComponent);
    void UpdateEquipmentForRole();
    
    // Material instances for customization
    UPROPERTY()
    UMaterialInstanceDynamic* SkinMaterial;

    UPROPERTY()
    UMaterialInstanceDynamic* ClothingMaterial;
};