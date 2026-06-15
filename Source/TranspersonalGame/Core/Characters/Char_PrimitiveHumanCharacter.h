#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "Char_PrimitiveHumanCharacter.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Child       UMETA(DisplayName = "Child"),
    Warrior     UMETA(DisplayName = "Warrior")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Light       UMETA(DisplayName = "Light Weathered"),
    Medium      UMETA(DisplayName = "Medium Weathered"),
    Dark        UMETA(DisplayName = "Dark Weathered"),
    Tanned      UMETA(DisplayName = "Sun Tanned"),
    Scarred     UMETA(DisplayName = "Battle Scarred")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float BodyScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasTribalPaint;

    FChar_TribalAppearance()
    {
        SkinTone = EChar_SkinTone::Medium;
        SkinColor = FLinearColor(0.4f, 0.25f, 0.15f, 1.0f);
        BodyScale = 1.0f;
        bHasScars = false;
        bHasTribalPaint = false;
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalEquipment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* SpearMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* ClothingMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    class UStaticMesh* ToolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasSpear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasClothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
    bool bHasTool;

    FChar_TribalEquipment()
    {
        SpearMesh = nullptr;
        ClothingMesh = nullptr;
        ToolMesh = nullptr;
        bHasSpear = true;
        bHasClothing = true;
        bHasTool = false;
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

    // Character Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    EChar_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FChar_TribalEquipment Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Character")
    FString TribalName;

    // Equipment Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* SpearComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ClothingComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* ToolComponent;

    // Dynamic Materials
    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* SkinMaterial;

    UPROPERTY(BlueprintReadOnly, Category = "Materials")
    class UMaterialInstanceDynamic* ClothingMaterial;

    // Atmospheric Lighting Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bRespondToAtmosphericLighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightingResponseIntensity;

    // Character Customization Functions
    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetTribalRole(EChar_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetSkinTone(EChar_SkinTone NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void SetBodyScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void ApplyTribalAppearance(const FChar_TribalAppearance& NewAppearance);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipSpear(class UStaticMesh* SpearMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipClothing(class UStaticMesh* ClothingMesh);

    UFUNCTION(BlueprintCallable, Category = "Character Customization")
    void EquipTool(class UStaticMesh* ToolMesh);

    // Atmospheric Lighting Response
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateAtmosphericLightingResponse(float TimeOfDay, float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetLightingResponseEnabled(bool bEnabled);

    // Character Behavior
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void PlayTribalAnimation(const FString& AnimationName);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetTribalStance(bool bCombatReady);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FLinearColor GetSkinColorForTone(EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FString GetTribalRoleName(EChar_TribalRole Role);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ApplyPresetAppearance(EChar_TribalRole Role);

private:
    void InitializeComponents();
    void SetupMaterials();
    void ConfigureForAtmosphericLighting();
    void AttachEquipment();
    void UpdateSkinMaterial();
    void UpdateClothingMaterial();
};