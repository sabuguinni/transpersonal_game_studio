#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "TribalCharacterSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Pale        UMETA(DisplayName = "Pale"),
    Fair        UMETA(DisplayName = "Fair"),
    Olive       UMETA(DisplayName = "Olive"),
    Bronze      UMETA(DisplayName = "Bronze"),
    Dark        UMETA(DisplayName = "Dark"),
    Deep        UMETA(DisplayName = "Deep")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole Role = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Bronze;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Height = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weight = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName = TEXT("Tribal Member");

    FChar_TribalAppearance()
    {
        Role = EChar_TribalRole::Hunter;
        SkinTone = EChar_SkinTone::Bronze;
        BodyType = EChar_BodyType::Athletic;
        bIsMale = true;
        Height = 1.75f;
        Weight = 70.0f;
        Age = 25;
        CharacterName = TEXT("Tribal Member");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ChestMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> LegsMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> FeetMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> AccessoryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ClothingName = TEXT("Basic Tribal");

    FChar_ClothingSet()
    {
        ClothingName = TEXT("Basic Tribal");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTribalCharacterSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UTribalCharacterSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    FChar_TribalAppearance CurrentAppearance;

    // Available clothing sets for different roles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TMap<EChar_TribalRole, FChar_ClothingSet> ClothingSets;

    // Skin tone materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TMap<EChar_SkinTone, TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    // Body scaling factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character System")
    TMap<EChar_BodyType, FVector> BodyScales;

    // Character generation and customization
    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplyTribalAppearance(const FChar_TribalAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void SetBodyType(EChar_BodyType NewBodyType);

    UFUNCTION(BlueprintCallable, Category = "Character System")
    FChar_TribalAppearance GenerateRandomAppearance();

    // Material application
    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplySkinMaterial();

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplyClothingMaterials();

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void ApplyBodyScaling();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Character System")
    USkeletalMeshComponent* GetCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Character System")
    void InitializeDefaultAppearance();

private:
    // Internal helper functions
    void SetupDefaultClothingSets();
    void SetupDefaultSkinMaterials();
    void SetupDefaultBodyScales();
    void ApplyMaterialToMeshSlot(UMaterialInterface* Material, int32 SlotIndex);
};