#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Char_TribalVariationSystem.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"), 
    Elder       UMETA(DisplayName = "Elder"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Scout       UMETA(DisplayName = "Scout")
};

USTRUCT(BlueprintType)
struct FChar_TribalVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    EChar_TribalArchetype Archetype = EChar_TribalArchetype::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    EBiomeType BiomeAdaptation = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    FString CharacterName = TEXT("Tribal_Character");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float SkinTone = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float BodyMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    TArray<FString> ClothingAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Variation")
    TArray<FString> AccessoryAssets;

    FChar_TribalVariation()
    {
        ClothingAssets.Empty();
        AccessoryAssets.Empty();
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_TribalVariationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_TribalVariationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    FChar_TribalVariation CurrentVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    TMap<EChar_TribalArchetype, float> ArchetypeSkillModifiers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal System")
    TMap<EBiomeType, FLinearColor> BiomeSkinTones;

public:
    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void ApplyTribalVariation(const FChar_TribalVariation& NewVariation);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    void GenerateRandomVariation(EBiomeType TargetBiome);

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FChar_TribalVariation GetCurrentVariation() const { return CurrentVariation; }

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    float GetArchetypeSkillModifier(EChar_TribalArchetype Archetype) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal System")
    FLinearColor GetBiomeSkinTone(EBiomeType Biome) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Tribal System")
    void InitializeTribalPresets();

private:
    void SetupArchetypeModifiers();
    void SetupBiomeSkinTones();
    void ApplyClothingVariation();
    void ApplyPhysicalVariation();
};