#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Materials/MaterialParameterCollection.h"
#include "SharedTypes.h"
#include "Char_BiomeLightingAdapter.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_BiomeLightingProfile
{
    GENERATED_BODY()

    // Rim lighting color for this biome
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    FLinearColor RimLightColor;

    // Rim lighting intensity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float RimLightIntensity;

    // Ambient character lighting boost
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float AmbientBoost;

    // Shadow softness multiplier
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float ShadowSoftness;

    // Subsurface scattering strength for skin
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float SubsurfaceStrength;

    FChar_BiomeLightingProfile()
        : RimLightColor(FLinearColor::White)
        , RimLightIntensity(1.0f)
        , AmbientBoost(0.1f)
        , ShadowSoftness(1.0f)
        , SubsurfaceStrength(0.5f)
    {}
};

/**
 * Character lighting adaptation component that adjusts character lighting
 * based on the current biome lighting conditions
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UChar_BiomeLightingAdapter : public UActorComponent
{
    GENERATED_BODY()

public:
    UChar_BiomeLightingAdapter();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Update character lighting based on current biome
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateCharacterLighting();

    // Get current biome based on world position
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    EBiomeType GetCurrentBiome() const;

    // Apply lighting profile for specific biome
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void ApplyBiomeLightingProfile(EBiomeType BiomeType);

protected:
    // Biome-specific lighting profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    TMap<EBiomeType, FChar_BiomeLightingProfile> BiomeLightingProfiles;

    // Character rim light component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character Lighting")
    class UPointLightComponent* CharacterRimLight;

    // Material parameter collection for character lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    class UMaterialParameterCollection* CharacterLightingMPC;

    // Current biome for transition tracking
    UPROPERTY(BlueprintReadOnly, Category = "Character Lighting")
    EBiomeType CurrentBiome;

    // Previous biome for smooth transitions
    UPROPERTY(BlueprintReadOnly, Category = "Character Lighting")
    EBiomeType PreviousBiome;

    // Transition progress (0.0 to 1.0)
    UPROPERTY(BlueprintReadOnly, Category = "Character Lighting")
    float TransitionProgress;

    // Transition speed
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Lighting")
    float TransitionSpeed;

private:
    // Initialize default biome lighting profiles
    void InitializeBiomeLightingProfiles();

    // Smoothly interpolate between lighting profiles
    FChar_BiomeLightingProfile InterpolateLightingProfiles(const FChar_BiomeLightingProfile& From, const FChar_BiomeLightingProfile& To, float Alpha) const;

    // Update material parameters
    void UpdateMaterialParameters(const FChar_BiomeLightingProfile& Profile);

    // Update rim light settings
    void UpdateRimLight(const FChar_BiomeLightingProfile& Profile);
};

#include "Char_BiomeLightingAdapter.generated.h"