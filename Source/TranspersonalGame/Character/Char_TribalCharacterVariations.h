#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterVariations.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalVariationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float SkinTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float Height;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float MuscleMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    FString TribalMarkings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    FString ClothingStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float ScarDensity;

    FChar_TribalVariationData()
    {
        SkinTone = 0.5f;
        BodyBuild = 0.5f;
        Height = 1.0f;
        MuscleMass = 0.6f;
        TribalMarkings = TEXT("None");
        ClothingStyle = TEXT("Basic");
        WeatheringLevel = 0.3f;
        ScarDensity = 0.2f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> ChestPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> LegPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> FootPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UStaticMesh> HeadPiece;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    TSoftObjectPtr<UMaterialInterface> ClothingMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    FString ClothingName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing")
    float DurabilityLevel;

    FChar_TribalClothingSet()
    {
        ClothingName = TEXT("Basic Hide");
        DurabilityLevel = 1.0f;
    }
};

/**
 * Character Artist Agent #9 - Tribal Character Variations System
 * Manages visual diversity for tribal NPCs and player customization options
 * Creates unique appearances through procedural variation of physical traits
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalCharacterVariations : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalCharacterVariations();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    TArray<FChar_TribalVariationData> TribalVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Clothing", meta = (AllowPrivateAccess = "true"))
    TArray<FChar_TribalClothingSet> ClothingSets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    float VariationSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    int32 MaxVariations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    bool bUseProceduralGeneration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    float SkinToneRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    float BodyBuildRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variations", meta = (AllowPrivateAccess = "true"))
    float HeightVariation;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    FChar_TribalVariationData GenerateRandomVariation();

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void ApplyVariationToCharacter(USkeletalMeshComponent* MeshComponent, const FChar_TribalVariationData& Variation);

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    FChar_TribalClothingSet GetRandomClothingSet();

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void ApplyClothingToCharacter(AActor* Character, const FChar_TribalClothingSet& ClothingSet);

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void InitializeVariationSystem();

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void CreateTribalVariationPresets();

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    FChar_TribalVariationData BlendVariations(const FChar_TribalVariationData& VariationA, const FChar_TribalVariationData& VariationB, float BlendFactor);

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void SetVariationSeed(float NewSeed);

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    TArray<FChar_TribalVariationData> GetAllVariations() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    void SaveVariationToFile(const FChar_TribalVariationData& Variation, const FString& FileName);

    UFUNCTION(BlueprintCallable, Category = "Tribal Variations")
    FChar_TribalVariationData LoadVariationFromFile(const FString& FileName);
};