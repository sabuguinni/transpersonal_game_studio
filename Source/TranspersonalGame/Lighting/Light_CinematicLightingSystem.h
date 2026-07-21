#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Light_CinematicLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_CinematicMood : uint8
{
    Dawn        UMETA(DisplayName = "Dawn - Soft Golden"),
    Midday      UMETA(DisplayName = "Midday - Harsh Bright"),
    Dusk        UMETA(DisplayName = "Dusk - Warm Orange"),
    Night       UMETA(DisplayName = "Night - Cool Blue"),
    Storm       UMETA(DisplayName = "Storm - Dark Dramatic"),
    Fire        UMETA(DisplayName = "Fire - Warm Flickering")
};

USTRUCT(BlueprintType)
struct FLight_CinematicLightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float ContrastBoost;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float SaturationMultiplier;

    FLight_CinematicLightingPreset()
    {
        PresetName = TEXT("Default");
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SunIntensity = 3.0f;
        SunRotation = FRotator(-45.0f, 135.0f, 0.0f);
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        ContrastBoost = 1.2f;
        SaturationMultiplier = 1.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_CinematicLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_CinematicLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CINEMATIC LIGHTING CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void SetCinematicMood(ELight_CinematicMood NewMood);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void ApplyLightingPreset(const FLight_CinematicLightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void CreateDramaticShadows(float ShadowIntensity = 1.5f);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void SetupThreePointLighting(FVector SubjectLocation);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void EnableVolumetricLighting(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void CreateAtmosphericPerspective(float FogStartDistance = 1000.0f);

    // === DYNAMIC LIGHTING EFFECTS ===

    UFUNCTION(BlueprintCallable, Category = "Dynamic Effects")
    void CreateFireLighting(FVector FireLocation, float Intensity = 800.0f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Effects")
    void AnimateFlickeringTorch(class APointLight* TorchLight);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Effects")
    void CreateLightningFlash(float Duration = 0.2f);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Effects")
    void TransitionToMood(ELight_CinematicMood TargetMood, float TransitionTime = 3.0f);

    // === SHADOW & CONTRAST CONTROL ===

    UFUNCTION(BlueprintCallable, Category = "Shadow Control")
    void EnhanceShadowDefinition(float ShadowBias = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Shadow Control")
    void SetContactShadows(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Shadow Control")
    void ConfigureRayTracedShadows(bool bEnable = false);

protected:
    // === LIGHTING PRESETS ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Presets")
    TMap<ELight_CinematicMood, FLight_CinematicLightingPreset> MoodPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    ELight_CinematicMood CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Settings")
    float TransitionSpeed;

    // === LIGHT REFERENCES ===

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ADirectionalLight* MainSunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class APostProcessVolume* CinematicPostProcess;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    TArray<class APointLight*> DynamicLights;

    // === ANIMATION STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    bool bIsTransitioning;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    float TransitionProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    ELight_CinematicMood TransitionTarget;

    UPROPERTY(BlueprintReadOnly, Category = "Animation State")
    FLight_CinematicLightingPreset TransitionStartPreset;

private:
    // === INTERNAL METHODS ===

    void InitializeLightingPresets();
    void FindLightingActors();
    void UpdateTransition(float DeltaTime);
    FLight_CinematicLightingPreset InterpolateLightingPresets(const FLight_CinematicLightingPreset& A, const FLight_CinematicLightingPreset& B, float Alpha);
    void ApplyPresetToActors(const FLight_CinematicLightingPreset& Preset);
    void CreateThreePointLightSetup(FVector SubjectLocation);

    // === TIMER HANDLES ===
    FTimerHandle FlickerTimerHandle;
    FTimerHandle LightningTimerHandle;
};