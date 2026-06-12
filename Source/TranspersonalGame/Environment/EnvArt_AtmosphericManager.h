#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/AmbientSound.h"
#include "TranspersonalGame/SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngleYaw = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAnglePitch = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float LightIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogInscatteringColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float VolumetricFogScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor VolumetricFogAlbedo = FLinearColor(0.9f, 0.8f, 0.6f, 1.0f);

    FEnvArt_AtmosphericSettings()
    {
        // Default constructor with initialized values
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_SoundTriggerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    FString SoundName = TEXT("DefaultAmbient");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float VolumeMultiplier = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    bool bAutoActivate = true;

    FEnvArt_SoundTriggerData()
    {
        // Default constructor
    }
};

/**
 * Environment Artist Atmospheric Manager
 * Manages atmospheric lighting, fog, and environmental audio for Cretaceous period ambiance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmospheric settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    // Sound trigger data array
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FEnvArt_SoundTriggerData> SoundTriggers;

    // References to atmospheric actors
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ADirectionalLight* DirectionalLightRef;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    class AExponentialHeightFog* HeightFogRef;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<class AAmbientSound*> AmbientSoundRefs;

    // Atmospheric control functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAmbientSoundTriggers();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericSettings(const FEnvArt_AtmosphericSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void RefreshAtmosphericActors();

    // Editor-only functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorSetupAtmosphere();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorClearAtmosphere();

private:
    // Internal helper functions
    void FindOrCreateDirectionalLight();
    void FindOrCreateHeightFog();
    void SetupDefaultSoundTriggers();

    // Time of day management
    UPROPERTY(EditAnywhere, Category = "Time")
    float CurrentTimeOfDay = 0.75f; // Golden hour default

    UPROPERTY(EditAnywhere, Category = "Time")
    bool bDynamicTimeOfDay = false;

    void UpdateTimeOfDay(float DeltaTime);
};