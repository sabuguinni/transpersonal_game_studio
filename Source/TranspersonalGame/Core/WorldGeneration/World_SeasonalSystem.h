#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Landscape/Landscape.h"
#include "FoliageType.h"
#include "InstancedFoliageActor.h"
#include "World_SeasonalSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_Season : uint8
{
    DrySeasonEarly UMETA(DisplayName = "Early Dry Season"),
    DrySeasonPeak UMETA(DisplayName = "Peak Dry Season"),
    WetSeasonEarly UMETA(DisplayName = "Early Wet Season"),
    WetSeasonPeak UMETA(DisplayName = "Peak Wet Season")
};

USTRUCT(BlueprintType)
struct FWorld_SeasonalParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHumidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float CloudCoverage = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FLinearColor VegetationTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Temperature")
    float AverageTemperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainfallProbability = 0.2f;

    FWorld_SeasonalParameters()
    {
        SunIntensity = 3.0f;
        SunColor = FLinearColor::White;
        AtmosphereHumidity = 0.5f;
        CloudCoverage = 0.3f;
        VegetationDensity = 1.0f;
        VegetationTint = FLinearColor::White;
        WaterLevel = 0.0f;
        AverageTemperature = 28.0f;
        RainfallProbability = 0.2f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_SeasonalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_SeasonalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Current seasonal state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal System")
    EWorld_Season CurrentSeason = EWorld_Season::DrySeasonEarly;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal System")
    float SeasonProgress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal System")
    float SeasonDuration = 300.0f; // 5 minutes per season in seconds

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal System")
    bool bAutoProgressSeasons = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal System")
    float TransitionSpeed = 1.0f;

    // Seasonal parameters for each season
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal Parameters")
    FWorld_SeasonalParameters DrySeasonEarlyParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal Parameters")
    FWorld_SeasonalParameters DrySeasonPeakParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal Parameters")
    FWorld_SeasonalParameters WetSeasonEarlyParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Seasonal Parameters")
    FWorld_SeasonalParameters WetSeasonPeakParams;

    // Environment references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    UMaterialParameterCollection* EnvironmentMPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment References")
    ALandscape* MainLandscape;

    // Current interpolated parameters
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FWorld_SeasonalParameters CurrentParameters;

    // Public methods
    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void SetSeason(EWorld_Season NewSeason);

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void AdvanceToNextSeason();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    EWorld_Season GetCurrentSeason() const { return CurrentSeason; }

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    float GetSeasonProgress() const { return SeasonProgress; }

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    FWorld_SeasonalParameters GetCurrentSeasonalParameters() const { return CurrentParameters; }

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void ApplySeasonalChanges();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void InitializeEnvironmentReferences();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void UpdateAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void UpdateVegetation();

    UFUNCTION(BlueprintCallable, Category = "Seasonal System")
    void UpdateWaterLevels();

private:
    void UpdateSeasonProgress(float DeltaTime);
    void InterpolateSeasonalParameters();
    FWorld_SeasonalParameters GetSeasonParameters(EWorld_Season Season) const;
    FWorld_SeasonalParameters LerpSeasonalParameters(const FWorld_SeasonalParameters& A, const FWorld_SeasonalParameters& B, float Alpha) const;
    void SetupDefaultSeasonalParameters();

    float SeasonTimer = 0.0f;
    bool bIsTransitioning = false;
    FWorld_SeasonalParameters PreviousParameters;
    FWorld_SeasonalParameters TargetParameters;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 30.0f; // 30 seconds for smooth transitions
};