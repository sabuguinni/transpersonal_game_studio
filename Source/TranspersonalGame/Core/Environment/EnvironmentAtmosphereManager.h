#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/WindDirectionalSource.h"
#include "../../SharedTypes.h"
#include "EnvironmentAtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FRotator SunRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float WindStrength;

    FEnvArt_BiomeAtmosphere()
    {
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 3.0f;
        SunRotation = FRotator(-15.0f, 45.0f, 0.0f);
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        WindStrength = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentAtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentAtmosphereManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Biome atmosphere configurations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    TMap<EBiomeType, FEnvArt_BiomeAtmosphere> BiomeAtmosphereSettings;

    // Current active atmosphere
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FEnvArt_BiomeAtmosphere CurrentAtmosphere;

    // Time of day system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    float DayDurationMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time System")
    bool bAutoAdvanceTime;

public:
    virtual void Tick(float DeltaTime) override;

    // Atmosphere control functions
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetMidDayLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetDuskLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateBiomeFog(EBiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateParticleEffects(EBiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetupAmbientSounds(EBiomeType BiomeType, FVector Location);

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void InitializeBiomeAtmospheres();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void ApplyCurrentAtmosphere();

protected:
    // Internal helper functions
    void InterpolateAtmosphere(const FEnvArt_BiomeAtmosphere& FromAtmosphere, const FEnvArt_BiomeAtmosphere& ToAtmosphere, float Alpha);
    void UpdateSunPosition();
    void UpdateFogSettings();
    FLinearColor CalculateTimeOfDayColor(float Hour);
    FRotator CalculateSunRotation(float Hour);
};