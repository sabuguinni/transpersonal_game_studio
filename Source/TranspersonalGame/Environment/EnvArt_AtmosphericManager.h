#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "SharedTypes.h"
#include "EnvArt_AtmosphericManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_AtmosphericType : uint8
{
    None UMETA(DisplayName = "None"),
    VolumericFog UMETA(DisplayName = "Volumetric Fog"),
    DustParticles UMETA(DisplayName = "Dust Particles"),
    PollenDrift UMETA(DisplayName = "Pollen Drift"),
    MorningMist UMETA(DisplayName = "Morning Mist"),
    HeatHaze UMETA(DisplayName = "Heat Haze")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    EEnvArt_AtmosphericType AtmosphericType = EEnvArt_AtmosphericType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    FLinearColor TintColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    float VisibilityDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric")
    bool bDynamicTimeOfDay = true;

    FEnvArt_AtmosphericSettings()
    {
        AtmosphericType = EEnvArt_AtmosphericType::None;
        Intensity = 1.0f;
        TintColor = FLinearColor::White;
        VisibilityDistance = 10000.0f;
        bDynamicTimeOfDay = true;
    }
};

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UExponentialHeightFogComponent* FogComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* ParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Settings")
    FEnvArt_AtmosphericSettings AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Integration")
    EBiomeType TargetBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float CurrentTimeOfDay = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float TimeSpeed = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetAtmosphericType(EEnvArt_AtmosphericType NewType);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void UpdateFogSettings(float NewIntensity, FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric")
    void SetBiomeSpecificAtmosphere(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Time")
    void SetTimeOfDay(float NewTime);

    UFUNCTION(BlueprintCallable, Category = "Time")
    FLinearColor GetCurrentSunColor() const;

private:
    void UpdateTimeOfDay(float DeltaTime);
    void ApplyBiomeAtmosphere();
    void UpdateParticleEffects();
    FLinearColor CalculateSunColor(float TimeOfDay) const;
    float CalculateFogDensity(float TimeOfDay) const;
};