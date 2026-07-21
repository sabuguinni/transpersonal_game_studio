#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Animation/AnimBlueprint.h"
#include "Char_PlayerVisualManager.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerArchetype : uint8
{
    Paleontologist      UMETA(DisplayName = "Paleontologist"),
    TribalLeader        UMETA(DisplayName = "Tribal Leader"),
    YoungScout          UMETA(DisplayName = "Young Scout"),
    ElderShaman         UMETA(DisplayName = "Elder Shaman"),
    PrehistoricWarrior  UMETA(DisplayName = "Prehistoric Warrior")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_PlayerVisualConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    EChar_PlayerArchetype Archetype = EChar_PlayerArchetype::Paleontologist;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    float MuscleDefinition = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    float WeatheringLevel = 0.3f;

    FChar_PlayerVisualConfig()
    {
        // Default to UE5 Mannequin
        CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple")));
        AnimationBlueprint = TSoftObjectPtr<UAnimBlueprint>(FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Animations/ABP_Quinn")));
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visual")
    FChar_PlayerVisualConfig CurrentConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visual")
    bool bAutoApplyOnBeginPlay = true;

    // Core visual functions
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void ApplyVisualConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void SetPlayerArchetype(EChar_PlayerArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void UpdateSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void UpdateClothingColor(FLinearColor NewClothingColor);

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void SetMuscleDefinition(float Definition);

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void SetWeatheringLevel(float Weathering);

    // Mesh and material management
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void LoadAndApplyMesh(const TSoftObjectPtr<USkeletalMesh>& MeshAsset);

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void CreateDynamicMaterials();

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    void ApplyArchetypePreset(EChar_PlayerArchetype Archetype);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    USkeletalMeshComponent* GetPlayerMeshComponent() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual")
    bool IsVisualConfigurationValid() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual", CallInEditor = true)
    void PreviewConfiguration();

protected:
    // Internal configuration methods
    void ConfigurePaleontologist();
    void ConfigureTribalLeader();
    void ConfigureYoungScout();
    void ConfigureElderShaman();
    void ConfigurePrehistoricWarrior();

    // Material parameter names
    static const FName SkinToneParameterName;
    static const FName ClothingColorParameterName;
    static const FName MuscleDefinitionParameterName;
    static const FName WeatheringParameterName;

private:
    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY()
    TObjectPtr<UMaterialInstanceDynamic> DynamicClothingMaterial;

    bool bMaterialsCreated = false;
};