#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/AtmosphericFog.h"
#include "SharedTypes.h"
#include "Light_ForestLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_ForestLightType : uint8
{
    AmbientCanopy,
    ShadowPatch,
    CaveEntrance,
    CaveFire,
    AtmosphericFog
};

USTRUCT(BlueprintType)
struct FLight_ForestLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    ELight_ForestLightType LightType = ELight_ForestLightType::AmbientCanopy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float Intensity = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    FLinearColor LightColor = FLinearColor(0.7f, 1.0f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float AttenuationRadius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float FlickerIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float FlickerSpeed = 1.0f;
};

USTRUCT(BlueprintType)
struct FLight_CaveLightingData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float BaseIntensity = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FlickerAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FlickerFrequency = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FireColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bIsLit = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_ForestLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_ForestLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Forest lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    TArray<FLight_ForestLightConfig> ForestLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float CanopyDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float ShadowIntensity = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Forest Lighting")
    float TimeOfDay = 12.0f; // 0-24 hours

    // Cave lighting configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightingData CaveLightingData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    TArray<APointLight*> CaveFireLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    TArray<ASpotLight*> CaveEntranceLights;

    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    AAtmosphericFog* ForestFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    // Lighting control functions
    UFUNCTION(BlueprintCallable, Category = "Forest Lighting")
    void CreateForestLighting(const FVector& ForestCenter, float ForestRadius);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateCaveLighting(const FVector& CaveLocation, float CaveDepth);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericFog(float Density, const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void SetTimeOfDay(float Hours);

    UFUNCTION(BlueprintCallable, Category = "Lighting Control")
    void UpdateCanopyLighting(float DensityFactor);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void ToggleCaveFireLights(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateCaveFireFlicker(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Forest Lighting")
    void ApplyWeatherLighting(EWeatherType WeatherType);

    // Lighting utility functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utils")
    float CalculateCanopyOcclusion(const FVector& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Utils")
    FLinearColor GetTimeOfDayColor(float TimeHours) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Utils")
    float GetShadowIntensityAtPosition(const FVector& Position) const;

    UFUNCTION(BlueprintCallable, Category = "Lighting Utils")
    bool IsPositionInCave(const FVector& Position) const;

private:
    // Internal lighting state
    float FlickerTimer = 0.0f;
    float CanopyOcclusionCache = 0.0f;
    bool bLightingSystemInitialized = false;

    // Internal update functions
    void UpdateForestAmbientLighting(float DeltaTime);
    void UpdateCaveFireEffects(float DeltaTime);
    void UpdateAtmosphericEffects(float DeltaTime);
    void CacheCanopyOcclusion();
    void InitializeLightingSystem();
    void CleanupLightingActors();
};