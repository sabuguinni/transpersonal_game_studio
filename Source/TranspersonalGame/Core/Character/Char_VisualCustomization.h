#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Materials/MaterialParameterCollection.h"
#include "Char_VisualCustomization.generated.h"

/**
 * Facial feature variation data for tribal characters
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_FacialFeatures
{
    GENERATED_BODY()

    /** Nose bridge height variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float NoseBridgeHeight = 0.0f;

    /** Jaw width variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float JawWidth = 0.0f;

    /** Cheekbone prominence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float CheekboneProminence = 0.0f;

    /** Eye spacing variation */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float EyeSpacing = 0.0f;

    /** Brow ridge prominence */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facial Features")
    float BrowRidgeProminence = 0.0f;

    FChar_FacialFeatures()
    {
        NoseBridgeHeight = 0.0f;
        JawWidth = 0.0f;
        CheekboneProminence = 0.0f;
        EyeSpacing = 0.0f;
        BrowRidgeProminence = 0.0f;
    }
};

/**
 * Body proportions for tribal character variations
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_BodyProportions
{
    GENERATED_BODY()

    /** Overall height scale */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float HeightScale = 1.0f;

    /** Torso length ratio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float TorsoLength = 1.0f;

    /** Arm length ratio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float ArmLength = 1.0f;

    /** Leg length ratio */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float LegLength = 1.0f;

    /** Shoulder width */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Body")
    float ShoulderWidth = 1.0f;

    FChar_BodyProportions()
    {
        HeightScale = 1.0f;
        TorsoLength = 1.0f;
        ArmLength = 1.0f;
        LegLength = 1.0f;
        ShoulderWidth = 1.0f;
    }
};

/**
 * Component for visual customization of tribal characters
 * Handles facial features, body proportions, and material variations
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_VisualCustomization : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_VisualCustomization();

    /** Current facial feature configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_FacialFeatures FacialFeatures;

    /** Current body proportion configuration */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    FChar_BodyProportions BodyProportions;

    /** Available skin tone variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FLinearColor> SkinTones;

    /** Available hair color variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FLinearColor> HairColors;

    /** Available eye color variations */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
    TArray<FLinearColor> EyeColors;

    /** Current selected skin tone index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Selection")
    int32 CurrentSkinToneIndex = 0;

    /** Current selected hair color index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Selection")
    int32 CurrentHairColorIndex = 0;

    /** Current selected eye color index */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Selection")
    int32 CurrentEyeColorIndex = 0;

    /** Randomize all visual features */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeAllFeatures();

    /** Randomize facial features only */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeFacialFeatures();

    /** Randomize body proportions only */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeBodyProportions();

    /** Randomize color selections */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeColors();

    /** Apply current customization to character mesh */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyCustomizationToMesh();

    /** Get current skin tone color */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Customization")
    FLinearColor GetCurrentSkinTone() const;

    /** Get current hair color */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Customization")
    FLinearColor GetCurrentHairColor() const;

    /** Get current eye color */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Customization")
    FLinearColor GetCurrentEyeColor() const;

    /** Set skin tone by index */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetSkinTone(int32 Index);

    /** Set hair color by index */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetHairColor(int32 Index);

    /** Set eye color by index */
    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SetEyeColor(int32 Index);

protected:
    virtual void BeginPlay() override;

    /** Initialize default color variations */
    void InitializeDefaultColors();

    /** Apply facial features to mesh */
    void ApplyFacialFeatures();

    /** Apply body proportions to mesh */
    void ApplyBodyProportions();

    /** Apply color variations to materials */
    void ApplyColorVariations();

    /** Get the character's skeletal mesh component */
    class USkeletalMeshComponent* GetCharacterMesh() const;
};