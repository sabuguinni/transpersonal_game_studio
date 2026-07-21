#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Char_TribalVisualSystem.generated.h"

USTRUCT(BlueprintType)
struct FChar_TribalAppearanceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 FacialStructure;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    int32 BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> Accessories;

    FChar_TribalAppearanceData()
    {
        CharacterName = TEXT("Unnamed Survivor");
        SkinTone = 0;
        FacialStructure = 0;
        BodyBuild = 0;
        ClothingStyle = TEXT("Basic Hide");
    }
};

USTRUCT(BlueprintType)
struct FChar_TribalClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString SetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> TorsoMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> LegsMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<USkeletalMesh> FeetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    float WearLevel;

    FChar_TribalClothingSet()
    {
        SetName = TEXT("Default Hide Set");
        WearLevel = 0.5f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalVisualSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalVisualSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Character appearance configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Visuals")
    FChar_TribalAppearanceData CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Visuals")
    TArray<FChar_TribalClothingSet> AvailableClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Visuals")
    TSoftObjectPtr<UDataTable> TribalAppearanceDataTable;

    // Skin and body materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<TSoftObjectPtr<UMaterialInterface>> TribalMarkingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BaseSkinMaterial;

    // Character diversity settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity", meta = (ClampMin = "0", ClampMax = "10"))
    int32 EthnicVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity", meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float BodyScaleVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diversity")
    bool bUseRandomizedAppearance;

    // Visual state tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    bool bIsAppearanceInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentHealthVisualState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    float CurrentFatigueVisualState;

    // Public functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void InitializeTribalAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void ApplyTribalAppearance(const FChar_TribalAppearanceData& AppearanceData);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void ApplyClothingSet(const FChar_TribalClothingSet& ClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void UpdateHealthVisuals(float HealthPercentage);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void UpdateFatigueVisuals(float FatiguePercentage);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void ApplyTribalMarkings(const TArray<FString>& MarkingPatterns);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    FChar_TribalAppearanceData GenerateRandomAppearance();

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetSkinTone(int32 SkinToneIndex);

    UFUNCTION(BlueprintCallable, Category = "Tribal Visuals")
    void SetBodyBuild(int32 BodyBuildIndex);

private:
    // Internal helper functions
    void LoadAppearanceAssets();
    void ConfigureMeshComponents();
    void ApplyMaterialParameters();
    void UpdateVisualEffects(float DeltaTime);
    
    // Component references
    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> OwnerMeshComponent;
    
    // Runtime state
    float VisualUpdateTimer;
    bool bNeedsVisualUpdate;
};