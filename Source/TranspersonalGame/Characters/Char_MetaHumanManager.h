#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Char_MetaHumanManager.generated.h"

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    WeatheredBronze     UMETA(DisplayName = "Weathered Bronze"),
    SunKissed          UMETA(DisplayName = "Sun Kissed"),
    AgedLeather        UMETA(DisplayName = "Aged Leather"),
    YouthfulOlive      UMETA(DisplayName = "Youthful Olive")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    WarriorMuscular    UMETA(DisplayName = "Warrior Muscular"),
    HunterAgile        UMETA(DisplayName = "Hunter Agile"),
    ElderWise          UMETA(DisplayName = "Elder Wise"),
    ScoutNimble        UMETA(DisplayName = "Scout Nimble")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_CharacterVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_SkinTone SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_BodyType BodyType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor SkinBaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float SkinRoughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float SkinSubsurfaceScattering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> FacialScars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bHasFacialHair;

    FChar_CharacterVariation()
    {
        CharacterName = TEXT("DefaultTribal");
        SkinTone = EChar_SkinTone::WeatheredBronze;
        BodyType = EChar_BodyType::WarriorMuscular;
        SkinBaseColor = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);
        SkinRoughness = 0.7f;
        SkinSubsurfaceScattering = 0.3f;
        bHasFacialHair = false;
    }
};

/**
 * MetaHuman Character Manager for creating and managing diverse prehistoric human characters
 * Handles character variations, skin tones, facial features, and material customization
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_MetaHumanManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_MetaHumanManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CHARACTER VARIATION SYSTEM ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variations")
    TArray<FChar_CharacterVariation> CharacterVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variations")
    int32 MaxCharacterVariations;

    // === METAHUMAN INTEGRATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class USkeletalMesh* BaseMetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class UMaterialInterface* BaseSkinMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    TArray<class UMaterialInstanceDynamic*> SkinMaterialInstances;

    // === CHARACTER SPAWNING ===
    
    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    class APawn* SpawnCharacterVariation(const FChar_CharacterVariation& Variation, FVector Location, FRotator Rotation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void CreateRandomCharacterVariation(FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Creation")
    void GenerateCharacterDiversity(int32 NumCharacters, float SpawnRadius);

    // === MATERIAL CUSTOMIZATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Material System")
    class UMaterialInstanceDynamic* CreateSkinMaterialInstance(EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void ApplySkinToneToCharacter(class USkeletalMeshComponent* MeshComponent, EChar_SkinTone SkinTone);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void ApplyWeatheringEffects(class UMaterialInstanceDynamic* MaterialInstance, float WeatheringIntensity);

    // === FACIAL CUSTOMIZATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Facial Features")
    void AddFacialScar(class USkeletalMeshComponent* MeshComponent, const FString& ScarType);

    UFUNCTION(BlueprintCallable, Category = "Facial Features")
    void SetFacialHair(class USkeletalMeshComponent* MeshComponent, bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Facial Features")
    void ApplyAgeingEffects(class UMaterialInstanceDynamic* MaterialInstance, float AgeIntensity);

    // === LIGHTING INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Lighting Integration")
    void OptimizeForCretaceousLighting(class USkeletalMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Lighting Integration")
    void ConfigureSubsurfaceScattering(class UMaterialInstanceDynamic* MaterialInstance, float Intensity);

protected:
    // === INTERNAL SYSTEMS ===
    
    void InitializeCharacterVariations();
    void LoadMetaHumanAssets();
    FChar_CharacterVariation GenerateRandomVariation();
    FLinearColor GetSkinColorForTone(EChar_SkinTone SkinTone);
    
private:
    bool bIsInitialized;
    int32 SpawnedCharacterCount;
};