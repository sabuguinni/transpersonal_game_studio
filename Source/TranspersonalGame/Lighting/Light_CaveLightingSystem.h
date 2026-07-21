#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/PointLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Light_CaveLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveLightType : uint8
{
    FireLight       UMETA(DisplayName = "Fire Light"),
    EntranceLight   UMETA(DisplayName = "Entrance Light"),
    AmbientLight    UMETA(DisplayName = "Ambient Light"),
    TorchLight      UMETA(DisplayName = "Torch Light")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    ELight_CaveLightType LightType = ELight_CaveLightType::FireLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float Intensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.7f, 0.47f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AttenuationRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bUseFlickering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FlickerIntensity = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FlickerSpeed = 2.0f;

    FLight_CaveLightConfig()
    {
        LightType = ELight_CaveLightType::FireLight;
        Intensity = 800.0f;
        LightColor = FLinearColor(1.0f, 0.7f, 0.47f, 1.0f);
        AttenuationRadius = 500.0f;
        bUseFlickering = true;
        FlickerIntensity = 0.2f;
        FlickerSpeed = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveAtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    FLinearColor FogColor = FLinearColor(0.47f, 0.39f, 0.31f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Atmosphere")
    float VolumetricFogScatteringDistribution = 0.2f;

    FLight_CaveAtmosphereConfig()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogColor = FLinearColor(0.47f, 0.39f, 0.31f, 1.0f);
        bEnableVolumetricFog = true;
        VolumetricFogScatteringDistribution = 0.2f;
    }
};

/**
 * Cave Lighting System Component
 * Manages dynamic lighting for cave interiors including firelight, entrance lighting,
 * atmospheric effects, and lighting transitions for prehistoric cave environments.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_CaveLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_CaveLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CAVE LIGHTING CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightConfig FireLightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightConfig EntranceLightConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveAtmosphereConfig AtmosphereConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bAutoSetupLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float LightingTransitionRadius = 1000.0f;

    // === LIGHTING ACTORS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    TArray<APointLight*> CaveLights;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    AExponentialHeightFog* CaveFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    APointLight* MainFireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cave Lighting")
    APointLight* EntranceLight;

    // === LIGHTING METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateFireLight(const FVector& Location, const FLight_CaveLightConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateEntranceLight(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateCaveAtmosphere(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateFlickeringLights(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetLightingIntensity(float IntensityMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void EnableCaveLighting(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void TransitionToInteriorLighting(float TransitionAlpha);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void TransitionToExteriorLighting(float TransitionAlpha);

    // === ATMOSPHERIC EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void UpdateCaveAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void SetFogDensity(float Density);

    UFUNCTION(BlueprintCallable, Category = "Cave Atmosphere")
    void SetFogColor(const FLinearColor& Color);

private:
    // === INTERNAL STATE ===
    float FlickerTime;
    float BaseIntensityMultiplier;
    bool bIsPlayerInCave;
    float CurrentTransitionAlpha;

    // === INTERNAL METHODS ===
    void InitializeLightingSystem();
    void CleanupLightingActors();
    float CalculateFlickerValue(float Time, float Speed, float Intensity) const;
    void UpdateLightTransition(float DeltaTime);
};