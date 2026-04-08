#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../Lighting/LightingMasterController.h"
#include "CharacterLightingIntegration.generated.h"

class UMetaHumanCharacterComponent;
class ALightingMasterController;

/**
 * Character Lighting Integration Component
 * 
 * Integrates characters with the dynamic lighting and atmosphere system.
 * Ensures characters respond realistically to time of day, weather, and emotional lighting.
 * 
 * This is where Caravaggio meets the Jurassic period.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterLightingIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterLightingIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core lighting integration
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void InitializeLightingIntegration();

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateCharacterLighting(float DeltaTime);

    // Dynamic material responses
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateSkinMaterialForLighting(const FLightingState& LightingState);

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateClothingMaterialsForLighting(const FLightingState& LightingState);

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateHairMaterialsForLighting(const FLightingState& LightingState);

    // Atmospheric effects on character
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void ApplyWeatherEffectsToCharacter(const FAtmosphericState& AtmosphericState);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void UpdateCharacterForTimeOfDay(float TimeOfDay);

    // Emotional lighting responses
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void RespondToEmotionalLighting(EEmotionalLightingMode EmotionalMode, float Intensity);

    // Shadow and rim lighting
    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateRimLighting(const FVector& LightDirection, const FLinearColor& LightColor);

    UFUNCTION(BlueprintCallable, Category = "Character Lighting")
    void UpdateSubsurfaceScattering(float Intensity);

protected:
    // Component references
    UPROPERTY()
    UMetaHumanCharacterComponent* CharacterComponent;

    UPROPERTY()
    ALightingMasterController* LightingController;

    // Material instances for lighting control
    UPROPERTY()
    UMaterialInstanceDynamic* SkinLightingMaterial;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> ClothingLightingMaterials;

    UPROPERTY()
    TArray<UMaterialInstanceDynamic*> HairLightingMaterials;

    // Lighting parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    float SkinSubsurfaceIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    float RimLightingIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Settings")
    float EmotionalResponseIntensity = 0.7f;

    // Time-based lighting variations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FLinearColor DawnSkinTint = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FLinearColor NoonSkinTint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FLinearColor DuskSkinTint = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Lighting")
    FLinearColor NightSkinTint = FLinearColor(0.3f, 0.4f, 0.6f, 1.0f);

    // Weather effects on materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float WetnessIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float DirtAccumulation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float SweatLevel = 0.0f;

private:
    // Internal state
    FLightingState CurrentLightingState;
    FAtmosphericState CurrentAtmosphericState;
    float LastUpdateTime = 0.0f;
    
    // Helper functions
    void CreateLightingMaterialInstances();
    void UpdateMaterialParameters();
    FLinearColor CalculateTimeBasedSkinTint(float TimeOfDay);
    float CalculateSubsurfaceIntensity(const FVector& LightDirection);
};