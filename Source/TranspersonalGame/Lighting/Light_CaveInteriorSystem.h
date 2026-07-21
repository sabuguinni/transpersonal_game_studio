#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Engine/DirectionalLight.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Light_CaveInteriorSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveFireConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Config")
    float BaseIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Config")
    float FlickerRange = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Config")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Config")
    FLinearColor FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Config")
    float AttenuationRadius = 800.0f;

    FLight_CaveFireConfig()
    {
        BaseIntensity = 2000.0f;
        FlickerRange = 400.0f;
        FlickerSpeed = 2.0f;
        FireColor = FLinearColor(1.0f, 0.4f, 0.1f, 1.0f);
        AttenuationRadius = 800.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveTorchConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Config")
    float BaseIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Config")
    float FlickerRange = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Config")
    float FlickerSpeed = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Config")
    FLinearColor TorchColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Config")
    float AttenuationRadius = 400.0f;

    FLight_CaveTorchConfig()
    {
        BaseIntensity = 800.0f;
        FlickerRange = 200.0f;
        FlickerSpeed = 3.0f;
        TorchColor = FLinearColor(1.0f, 0.6f, 0.2f, 1.0f);
        AttenuationRadius = 400.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveAtmosphereConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    FLinearColor FogInscatteringColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Config")
    float HumidityLevel = 0.7f;

    FLight_CaveAtmosphereConfig()
    {
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        FogInscatteringColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);
        VolumetricFogScatteringDistribution = 0.2f;
        VolumetricFogAlbedo = FLinearColor(0.9f, 0.9f, 0.9f, 1.0f);
        HumidityLevel = 0.7f;
    }
};

/**
 * Cave Interior Lighting System for Prehistoric Habitation
 * Manages dynamic fire pit lighting, torch flickering, atmospheric fog, and mood lighting
 * Designed for realistic Stone Age cave environments with proper light interaction
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_CaveInteriorSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_CaveInteriorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fire Lighting")
    class UPointLightComponent* FirePitLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entrance Lighting")
    class USpotLightComponent* CaveEntranceLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ambient Lighting")
    class UDirectionalLightComponent* RimLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Post Process")
    class UPostProcessComponent* CavePostProcess;

    // Torch Lights Array
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Torch Lighting")
    TArray<class UPointLightComponent*> TorchLights;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    FLight_CaveFireConfig FireConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    FLight_CaveTorchConfig TorchConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    FLight_CaveAtmosphereConfig AtmosphereConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    int32 NumberOfTorches = 3;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    float TorchPlacementRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    bool bEnableFlickering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting Config")
    bool bEnablePostProcessing = true;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float CurrentFireIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    TArray<float> CurrentTorchIntensities;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    float FlickerTime;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime State")
    bool bIsSystemActive;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void ActivateCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void DeactivateCaveLighting();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetFireIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetTorchIntensity(int32 TorchIndex, float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetAtmosphericFogDensity(float NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetCaveHumidity(float HumidityLevel);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateLightingForTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateTorchAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void RemoveTorchAtIndex(int32 TorchIndex);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Cave Lighting")
    void RegenerateCaveLighting();

protected:
    // Internal Methods
    void InitializeFirePitLighting();
    void InitializeCaveEntranceLighting();
    void InitializeTorchLighting();
    void InitializeAtmosphericFog();
    void InitializeRimLighting();
    void InitializePostProcessing();

    void UpdateFlickeringLights(float DeltaTime);
    void UpdateFireFlicker(float DeltaTime);
    void UpdateTorchFlicker(float DeltaTime);
    void UpdateAtmosphericEffects(float DeltaTime);

    float CalculateFlickerIntensity(float BaseIntensity, float FlickerRange, float FlickerSpeed, float TimeOffset);
    FLinearColor CalculateFlickerColor(const FLinearColor& BaseColor, float FlickerIntensity);

    void ApplyCavePostProcessSettings();
    void UpdateVolumetricFogSettings();

    // Timer Handles
    FTimerHandle FlickerUpdateTimer;
    FTimerHandle AtmosphereUpdateTimer;

private:
    // Internal state tracking
    float FireFlickerOffset;
    TArray<float> TorchFlickerOffsets;
    float AtmosphereUpdateTime;
    bool bComponentsInitialized;
};