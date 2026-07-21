#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalVariant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    FString VariantName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    USkeletalMesh* CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    TArray<UMaterialInterface*> SkinMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    TArray<UStaticMesh*> Equipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    FLinearColor WarPaintColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variant")
    float SurvivalExperience;

    FChar_TribalVariant()
    {
        VariantName = TEXT("Default_Tribal");
        CharacterMesh = nullptr;
        WarPaintColor = FLinearColor::Red;
        SurvivalExperience = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_BiomeAdaptation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptation")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptation")
    float TemperatureResistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptation")
    float HumidityTolerance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptation")
    TArray<FString> PreferredEquipment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptation")
    float MovementSpeedModifier;

    FChar_BiomeAdaptation()
    {
        BiomeType = EBiomeType::Savanna;
        TemperatureResistance = 0.5f;
        HumidityTolerance = 0.5f;
        MovementSpeedModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UChar_TribalCharacterManager : public UObject
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variants")
    TArray<FChar_TribalVariant> TribalVariants;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Adaptations")
    TArray<FChar_BiomeAdaptation> BiomeAdaptations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    int32 MaxCharactersPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Generation")
    float CharacterSpawnRadius;

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    FChar_TribalVariant GetVariantForBiome(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void GenerateTribalCharacters(UWorld* World, const TArray<FVector>& BiomeLocations);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    ACharacter* SpawnTribalCharacter(UWorld* World, const FVector& Location, const FChar_TribalVariant& Variant);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void ApplyBiomeAdaptation(ACharacter* Character, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void UpdateCharacterAppearance(ACharacter* Character, const FChar_TribalVariant& Variant);

private:
    void InitializeDefaultVariants();
    void InitializeBiomeAdaptations();
    FChar_TribalVariant CreateVariant(const FString& Name, const FLinearColor& WarPaint, float Experience);
};