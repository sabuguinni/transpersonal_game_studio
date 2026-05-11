#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Char_PlayerVisualSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerArchetype : uint8
{
    Paleontologist      UMETA(DisplayName = "Paleontologist"),
    TribalSurvivor      UMETA(DisplayName = "Tribal Survivor"),
    TribalElder         UMETA(DisplayName = "Tribal Elder"),
    TribalScout         UMETA(DisplayName = "Tribal Scout"),
    TribalCraftsperson  UMETA(DisplayName = "Tribal Craftsperson")
};

USTRUCT(BlueprintType)
struct FChar_PlayerVisualConfig
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
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    FLinearColor ClothingColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    float WeatheringIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Config")
    float AgeProgression = 0.3f;

    FChar_PlayerVisualConfig()
    {
        CharacterMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath("/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple"));
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_PlayerVisualSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_PlayerVisualSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core visual configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Visual System")
    FChar_PlayerVisualConfig VisualConfig;

    // Character mesh component reference
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Visual System")
    TObjectPtr<USkeletalMeshComponent> CharacterMeshComponent;

    // Dynamic material instances
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Visual System")
    TObjectPtr<UMaterialInstanceDynamic> DynamicSkinMaterial;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Visual System")
    TObjectPtr<UMaterialInstanceDynamic> DynamicClothingMaterial;

    // Visual configuration methods
    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void ApplyVisualConfiguration(const FChar_PlayerVisualConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void SetPlayerArchetype(EChar_PlayerArchetype NewArchetype);

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void UpdateSkinTone(const FLinearColor& NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void UpdateClothingColor(const FLinearColor& NewClothingColor);

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void SetWeatheringIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void SetAgeProgression(float Age);

    // Mesh and material setup
    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void InitializeCharacterMesh();

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void CreateDynamicMaterials();

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    void ApplyMaterialParameters();

    // Archetype-specific configurations
    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    FChar_PlayerVisualConfig GetPaleontologistConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    FChar_PlayerVisualConfig GetTribalSurvivorConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    FChar_PlayerVisualConfig GetTribalElderConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    FChar_PlayerVisualConfig GetTribalScoutConfig() const;

    UFUNCTION(BlueprintCallable, Category = "Player Visual System")
    FChar_PlayerVisualConfig GetTribalCraftspersonConfig() const;

    // Editor utilities
    UFUNCTION(CallInEditor, Category = "Player Visual System")
    void RefreshVisuals();

    UFUNCTION(CallInEditor, Category = "Player Visual System")
    void RandomizeAppearance();

private:
    // Internal setup methods
    void FindCharacterMeshComponent();
    void LoadDefaultAssets();
    void SetupMaterialParameters();
    
    // Archetype configuration helpers
    void ConfigureForPaleontologist();
    void ConfigureForTribalSurvivor();
    void ConfigureForTribalElder();
    void ConfigureForTribalScout();
    void ConfigureForTribalCraftsperson();
};