#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/VolumetricFogComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "EnvArt_AtmosphericSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAngle = -15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunAzimuth = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bEnableVolumetricShadows = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_FogSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogAlbedo = FLinearColor(0.9f, 0.95f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ExtinctionScale = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float ScatteringScale = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FVector FogScale = FVector(20.0f, 20.0f, 5.0f);
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float PitchMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bAutoActivate = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_AtmosphericSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_AtmosphericSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FEnvArt_LightingSettings LightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    FEnvArt_FogSettings FogSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric System")
    TArray<FEnvArt_AudioZone> AudioZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UDirectionalLightComponent* DirectionalLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UVolumetricFogComponent* VolumetricFogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UAudioComponent*> AudioComponents;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void ApplyGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void SetupVolumetricFog(const FVector& Location, const FVector& Scale);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void CreateAudioZone(const FEnvArt_AudioZone& AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric System")
    void UpdateAtmosphericSettings();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Atmospheric System")
    void RefreshAtmosphericSystem();

private:
    void InitializeComponents();
    void SetupDefaultSettings();
};