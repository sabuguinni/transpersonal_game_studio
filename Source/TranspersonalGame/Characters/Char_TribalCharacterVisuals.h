#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_TribalCharacterVisuals.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Elder       UMETA(DisplayName = "Tribal Elder"),
    Scout       UMETA(DisplayName = "Young Scout"), 
    Healer      UMETA(DisplayName = "Tribal Healer"),
    Warrior     UMETA(DisplayName = "Tribal Warrior"),
    Child       UMETA(DisplayName = "Tribal Child")
};

USTRUCT(BlueprintType)
struct FChar_TribalAppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalArchetype Archetype = EChar_TribalArchetype::Scout;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Age = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    float Weathering = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasWarPaint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    bool bHasScars = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> ClothingElements;

    FChar_TribalAppearanceData()
    {
        ClothingElements.Add(TEXT("Hide Vest"));
        ClothingElements.Add(TEXT("Bone Ornaments"));
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalMeshVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<USkeletalMesh> BaseMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    TArray<TSoftObjectPtr<UStaticMesh>> AccessoryMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    FString VariantName = TEXT("Default");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
    float LODBias = 1.0f;
};

/**
 * Component responsible for managing visual appearance of tribal characters
 * Handles mesh variants, materials, and appearance customization for Cretaceous period NPCs
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCharacterVisuals : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterVisuals();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core appearance management
    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void ApplyTribalAppearance(const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetTribalArchetype(EChar_TribalArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void ApplyWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void AddWarPaint(FLinearColor PaintColor);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void AddBattleScars(int32 NumScars);

    // Mesh and material management
    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetMeshVariant(const FChar_TribalMeshVariant& MeshVariant);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void AttachAccessory(UStaticMesh* AccessoryMesh, FName SocketName);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void UpdateClothing(const TArray<FString>& ClothingItems);

    // Lighting integration
    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void UpdateForLightingConditions(float SunIntensity, FLinearColor AmbientColor);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetSubsurfaceScattering(float ScatteringAmount);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Tribal Visuals")
    FChar_TribalAppearanceData GetCurrentAppearance() const { return CurrentAppearance; }

    UFUNCTION(BlueprintPure, Category = "Tribal Visuals")
    EChar_TribalArchetype GetArchetype() const { return CurrentAppearance.Archetype; }

    UFUNCTION(BlueprintPure, Category = "Tribal Visuals")
    bool IsAppearanceValid() const;

protected:
    // Appearance data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance", meta = (AllowPrivateAccess = "true"))
    FChar_TribalAppearanceData CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh Variants", meta = (AllowPrivateAccess = "true"))
    TMap<EChar_TribalArchetype, FChar_TribalMeshVariant> ArchetypeMeshes;

    // Material instances
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    class UMaterialParameterCollection* TribalMaterialParams;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials", meta = (AllowPrivateAccess = "true"))
    TArray<class UMaterialInstanceDynamic*> DynamicMaterials;

    // Component references
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    TArray<class UStaticMeshComponent*> AccessoryComponents;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    bool bAutoUpdateLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float MaterialUpdateInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    int32 MaxAccessories = 8;

private:
    // Internal methods
    void InitializeArchetypeMeshes();
    void CreateDynamicMaterials();
    void UpdateMaterialParameters();
    void ValidateAppearanceData();
    FChar_TribalMeshVariant GetMeshVariantForArchetype(EChar_TribalArchetype Archetype) const;
    void CleanupAccessories();

    // Timing
    float LastMaterialUpdate = 0.0f;
    bool bInitialized = false;
};