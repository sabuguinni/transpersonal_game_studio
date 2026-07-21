#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "SharedTypes.h"
#include "Char_MetaHumanCustomizationSystem.generated.h"

class UMaterialInterface;
class USkeletalMesh;
class UAnimBlueprint;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> HeadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<USkeletalMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UMaterialInterface> SkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TSoftObjectPtr<UAnimBlueprint> AnimationBlueprint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    EChar_TribalVariation TribalType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float AgeModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float BodyMassModifier;

    FChar_MetaHumanPreset()
    {
        PresetName = TEXT("Default");
        TribalType = EChar_TribalVariation::Hunter;
        AgeModifier = 0.5f;
        BodyMassModifier = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_FacialCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    float EyeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    float NoseSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    float MouthWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    float JawWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    float CheekboneHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    FLinearColor SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial")
    FLinearColor HairColor;

    FChar_FacialCustomization()
    {
        EyeSize = 0.5f;
        NoseSize = 0.5f;
        MouthWidth = 0.5f;
        JawWidth = 0.5f;
        CheekboneHeight = 0.5f;
        SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_BodyCustomization
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float MuscleDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float BodyFat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    bool bHasBattleScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    bool bHasTribalTattoos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    bool bHasRitualMarkings;

    FChar_BodyCustomization()
    {
        Height = 0.5f;
        Weight = 0.5f;
        MuscleDefinition = 0.6f;
        BodyFat = 0.3f;
        bHasBattleScars = false;
        bHasTribalTattoos = false;
        bHasRitualMarkings = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_MetaHumanCustomizationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_MetaHumanCustomizationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FChar_MetaHumanPreset CurrentPreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FChar_FacialCustomization FacialSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FChar_BodyCustomization BodySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<FChar_MetaHumanPreset> AvailablePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    UDataTable* MetaHumanPresetsTable;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* CharacterMesh;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyMetaHumanPreset(const FChar_MetaHumanPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyFacialCustomization(const FChar_FacialCustomization& FacialSettings);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void ApplyBodyCustomization(const FChar_BodyCustomization& BodySettings);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void SetTribalVariation(EChar_TribalVariation TribalType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    FChar_MetaHumanPreset GetRandomPresetForTribalType(EChar_TribalVariation TribalType);

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    void LoadMetaHumanAssets();

    UFUNCTION(BlueprintCallable, Category = "MetaHuman")
    bool IsMetaHumanSystemReady() const;

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    FChar_MetaHumanPreset GetCurrentPreset() const { return CurrentPreset; }

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    FChar_FacialCustomization GetFacialSettings() const { return FacialSettings; }

    UFUNCTION(BlueprintPure, Category = "MetaHuman")
    FChar_BodyCustomization GetBodySettings() const { return BodySettings; }

private:
    void InitializeComponent();
    void LoadPresetData();
    void ApplyMaterialParameters();
    void UpdateMeshComponents();
    
    bool bIsInitialized;
    bool bMetaHumanAssetsLoaded;
};