#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Char_PlayerCustomization.generated.h"

UENUM(BlueprintType)
enum class EChar_PlayerGender : uint8
{
    Male        UMETA(DisplayName = "Male"),
    Female      UMETA(DisplayName = "Female")
};

UENUM(BlueprintType)
enum class EChar_PlayerBuild : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Average     UMETA(DisplayName = "Average"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Heavy       UMETA(DisplayName = "Heavy")
};

USTRUCT(BlueprintType)
struct FChar_PlayerFace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float FaceWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float JawSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float CheekboneHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float NoseSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float EyeSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    float BrowRidge;

    FChar_PlayerFace()
    {
        FaceWidth = 0.5f;
        JawSize = 0.5f;
        CheekboneHeight = 0.5f;
        NoseSize = 0.5f;
        EyeSize = 0.5f;
        BrowRidge = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FChar_PlayerHair
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 HairStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FLinearColor HairColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    float HairLength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    bool bHasBeard;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    int32 BeardStyle;

    FChar_PlayerHair()
    {
        HairStyle = 0;
        HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);
        HairLength = 0.5f;
        bHasBeard = false;
        BeardStyle = 0;
    }
};

USTRUCT(BlueprintType)
struct FChar_PlayerAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EChar_PlayerGender Gender;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    EChar_PlayerBuild BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Basic")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    float SkinRoughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Skin")
    float TanLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Eyes")
    FLinearColor EyeColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Face")
    FChar_PlayerFace FaceFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hair")
    FChar_PlayerHair HairSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scars")
    TArray<int32> ScarsAndMarks;

    FChar_PlayerAppearance()
    {
        Gender = EChar_PlayerGender::Male;
        BodyBuild = EChar_PlayerBuild::Average;
        Height = 1.0f;
        SkinTone = 0.5f;
        SkinRoughness = 0.3f;
        TanLevel = 0.4f;
        EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_PlayerCustomization : public UObject
{
    GENERATED_BODY()

public:
    UChar_PlayerCustomization();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Appearance")
    FChar_PlayerAppearance CurrentAppearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TArray<FChar_PlayerAppearance> AppearancePresets;

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyAppearanceToCharacter(class ACharacter* TargetCharacter);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void SaveAppearancePreset(const FString& PresetName, const FChar_PlayerAppearance& Appearance);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    bool LoadAppearancePreset(const FString& PresetName, FChar_PlayerAppearance& OutAppearance);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void RandomizeAppearance();

    UFUNCTION(BlueprintCallable, Category = "Customization")
    void ApplyTribalPreset(int32 PresetIndex);

    UFUNCTION(BlueprintCallable, Category = "Customization")
    TArray<FString> GetAvailablePresetNames() const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateAppearance(const FChar_PlayerAppearance& Appearance) const;

protected:
    UFUNCTION()
    void InitializeTribalPresets();

    UFUNCTION()
    void ApplyFaceFeatures(class USkeletalMeshComponent* MeshComponent, const FChar_PlayerFace& FaceData);

    UFUNCTION()
    void ApplyHairSettings(class USkeletalMeshComponent* MeshComponent, const FChar_PlayerHair& HairData);

    UFUNCTION()
    void ApplyBodyBuild(class USkeletalMeshComponent* MeshComponent, EChar_PlayerBuild BuildType);

    UFUNCTION()
    FChar_PlayerAppearance GenerateRandomTribalAppearance();

private:
    UPROPERTY()
    TMap<FString, FChar_PlayerAppearance> SavedPresets;
};