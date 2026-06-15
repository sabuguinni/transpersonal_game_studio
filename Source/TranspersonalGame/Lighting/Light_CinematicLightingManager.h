#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SpotLight.h"
#include "Engine/PointLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "TranspersonalGame.h"
#include "Light_CinematicLightingManager.generated.h"

UENUM(BlueprintType)
enum class ELight_LightingZone : uint8
{
    Ambient         UMETA(DisplayName = "Ambient Zone"),
    Dramatic        UMETA(DisplayName = "Dramatic Zone"),
    Mysterious      UMETA(DisplayName = "Mysterious Zone"),
    Warm            UMETA(DisplayName = "Warm Zone"),
    Cool            UMETA(DisplayName = "Cool Zone")
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn            UMETA(DisplayName = "Dawn"),
    Morning         UMETA(DisplayName = "Morning"),
    Midday          UMETA(DisplayName = "Midday"),
    Afternoon       UMETA(DisplayName = "Afternoon"),
    GoldenHour      UMETA(DisplayName = "Golden Hour"),
    Dusk            UMETA(DisplayName = "Dusk"),
    Night           UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_LightingPreset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    FString PresetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    FLinearColor DirectionalLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    float DirectionalLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    FRotator DirectionalLightRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Preset")
    float VolumetricScattering;

    FLight_LightingPreset()
    {
        PresetName = TEXT("Default");
        DirectionalLightColor = FLinearColor(1.0f, 0.9f, 0.8f, 1.0f);
        DirectionalLightIntensity = 3.5f;
        DirectionalLightRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogColor = FLinearColor(0.8f, 0.7f, 0.6f, 1.0f);
        FogDensity = 0.02f;
        VolumetricScattering = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CinematicZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Zone")
    ELight_LightingZone ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Zone")
    TArray<AActor*> ZoneLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Zone")
    bool bIsActive;

    FLight_CinematicZone()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        ZoneType = ELight_LightingZone::Ambient;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CinematicLightingManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_CinematicLightingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Lighting System Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    ELight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float TimeOfDayProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    bool bEnableDynamicTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    float DayDurationMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    TArray<FLight_LightingPreset> TimeOfDayPresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cinematic Lighting")
    TArray<FLight_CinematicZone> CinematicZones;

    // Volumetric Fog Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float BaseFogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FLinearColor FogInscatteringColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricFogScatteringDistribution;

    // Light References
    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    AExponentialHeightFog* MainFogActor;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    TArray<ASpotLight*> CinematicSpotLights;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    TArray<APointLight*> CinematicPointLights;

public:
    // Lighting Control Functions
    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void SetTimeOfDay(ELight_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void SetTimeOfDayProgress(float Progress);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void ApplyLightingPreset(const FLight_LightingPreset& Preset);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void CreateCinematicZone(FVector Center, float Radius, ELight_LightingZone ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void UpdateCinematicZone(int32 ZoneIndex, bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Lighting")
    void SetVolumetricFogProperties(float Density, FLinearColor InscatteringColor, float ScatteringDistribution);

    // Cinematic Lighting Effects
    UFUNCTION(BlueprintCallable, Category = "Cinematic Effects")
    void CreateDramaticRimLighting(FVector TargetLocation, FLinearColor LightColor, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Effects")
    void CreateAtmosphericPointLight(FVector Location, FLinearColor Color, float Intensity, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Cinematic Effects")
    void CreateVolumetricSpotlight(FVector Location, FRotator Rotation, FLinearColor Color, float Intensity);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void FindAndCacheLightReferences();

    UFUNCTION(BlueprintCallable, Category = "Lighting Utilities")
    void ValidateLightingSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void InitializeDefaultLightingPresets();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void CreateTestCinematicZones();

protected:
    // Internal Functions
    void UpdateTimeOfDay(float DeltaTime);
    void InterpolateLightingPresets(const FLight_LightingPreset& PresetA, const FLight_LightingPreset& PresetB, float Alpha);
    void UpdateVolumetricFog();
    void UpdateCinematicZones();
    FLight_LightingPreset GetPresetForTimeOfDay(ELight_TimeOfDay TimeOfDay);

private:
    float TimeAccumulator;
    bool bLightingSystemInitialized;
};