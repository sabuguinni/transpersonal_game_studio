#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SharedTypes.h"
#include "Char_MetaHumanManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> HeadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> HairMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> EyeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float ScarIntensity;

    FChar_MetaHumanPreset()
    {
        PresetName = TEXT("Default");
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        HairColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        EyeColor = FLinearColor(0.2f, 0.4f, 0.6f, 1.0f);
        WeatheringLevel = 0.5f;
        ScarIntensity = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<FChar_MetaHumanPreset> TribalWarriorPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<FChar_MetaHumanPreset> SurvivorPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman Presets")
    TArray<FChar_MetaHumanPreset> ElderPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Materials")
    TArray<UMaterialInstanceDynamic*> DynamicMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Meshes")
    USkeletalMeshComponent* BodyMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Meshes")
    USkeletalMeshComponent* HeadMeshComponent;

public:
    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyPreset(const FChar_MetaHumanPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void RandomizeAppearance(EChar_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetSkinTone(FLinearColor NewSkinTone);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetHairColor(FLinearColor NewHairColor);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetEyeColor(FLinearColor NewEyeColor);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetWeatheringLevel(float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetScarIntensity(float ScarIntensity);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    FChar_MetaHumanPreset GetRandomPreset(EChar_CharacterType CharacterType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void InitializeCharacterMeshes(USkeletalMeshComponent* InBodyMesh, USkeletalMeshComponent* InHeadMesh);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void CreateDynamicMaterials();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void UpdateMaterialParameters();

private:
    void InitializeDefaultPresets();
    void SetupTribalWarriorPresets();
    void SetupSurvivorPresets();
    void SetupElderPresets();
    
    FLinearColor GenerateRandomSkinTone();
    FLinearColor GenerateRandomHairColor();
    FLinearColor GenerateRandomEyeColor();
};