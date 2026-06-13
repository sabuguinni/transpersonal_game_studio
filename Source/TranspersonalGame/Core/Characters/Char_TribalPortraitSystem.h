#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Char_TribalPortraitSystem.generated.h"

// TODO_ASSET_GENERATION_FAILED: DALL-E API failed for tribal portraits
// This system generates diverse tribal character appearances procedurally

/**
 * System for generating diverse tribal character portraits and appearances
 * Creates variation in facial features, scars, jewelry, and clothing
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalPortraitSystem : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalPortraitSystem();

protected:
    virtual void BeginPlay() override;

    // Portrait generation components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait System")
    UStaticMeshComponent* PortraitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portrait System")
    UMaterialInstanceDynamic* FaceMaterial;

    // Character variation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    TArray<FString> TribalNames;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    TArray<FLinearColor> SkinTones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation")
    TArray<FLinearColor> HairColors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ScarVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AgeVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Variation", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringIntensity;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void GenerateRandomTribalPortrait();

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void GenerateElderPortrait();

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void GenerateYoungHunterPortrait();

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void GenerateWarriorPortrait();

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void GenerateShamanPortrait();

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    FString GetRandomTribalName() const;

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void ApplyScarPattern(int32 PatternType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Portrait Generation")
    void ApplyTribalMarkings(int32 MarkingType);

private:
    void InitializePortraitSystem();
    void SetupMaterialParameters();
    FLinearColor GetRandomSkinTone() const;
    FLinearColor GetRandomHairColor() const;
    float GetRandomScarIntensity() const;
    float GetRandomAgeValue() const;
};

/*
TRIBAL CHARACTER PORTRAIT SPECIFICATIONS:

FACIAL VARIATIONS:
- 5 distinct age groups (child, young adult, mature, elder, ancient)
- 8 skin tone variations (light tan to deep bronze)
- 6 hair color options (black, dark brown, brown, auburn, gray, white)
- 4 facial structure types (lean, average, broad, weathered)

SCAR PATTERNS:
- Battle scars: diagonal cuts, claw marks, bite wounds
- Ritual scars: geometric patterns, tribal symbols
- Survival scars: burn marks, frostbite, sun damage
- Honor scars: ceremonial cuts, coming-of-age marks

TRIBAL MARKINGS:
- Face paint: war paint, ceremonial designs, camouflage
- Tattoos: tribal symbols, animal totems, achievement marks
- Piercings: bone, stone, metal ornaments
- Jewelry: necklaces, earrings, nose rings

CLOTHING VARIATIONS:
- Hunter: practical leather, minimal decoration
- Warrior: reinforced hide, battle trophies
- Elder: ceremonial robes, status symbols
- Shaman: ritual garments, mystical accessories
- Crafter: work clothes, tool belts

EXPRESSION TYPES:
- Stoic: weathered, experienced, calm
- Alert: vigilant, ready for danger
- Wise: knowing, contemplative
- Fierce: aggressive, intimidating
- Friendly: welcoming, trustworthy
*/