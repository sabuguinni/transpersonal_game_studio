#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/AtmosphericFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "Light_AtmosphereManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeAtmosphere
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FVector BiomeCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    FLinearColor AtmosphericColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Atmosphere")
    float LightIntensity;

    FLight_BiomeAtmosphere()
    {
        BiomeName = TEXT("Unknown");
        BiomeCenter = FVector::ZeroVector;
        AtmosphericColor = FLinearColor::White;
        FogDensity = 0.1f;
        LightIntensity = 2.0f;
    }
};

UENUM(BlueprintType)
enum class ELight_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_AtmosphereManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_AtmosphereManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TObjectPtr<UDirectionalLightComponent> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TObjectPtr<UAtmosphericFogComponent> AtmosphericFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FLight_BiomeAtmosphere> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    ELight_TimeOfDay CurrentPeriod;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    float CretaceousTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Lighting")
    FLinearColor CretaceousAmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    bool bEnableVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    bool bEnableLumenGI;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeBiomeAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateTimeOfDay(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetCretaceousLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ConfigureLumenSettings();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SpawnBiomeAtmosphericLights();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FLight_BiomeAtmosphere GetBiomeAtmosphereByLocation(FVector Location);

private:
    UPROPERTY()
    TArray<TObjectPtr<UPointLightComponent>> BiomeAtmosphericLights;

    void UpdateSunPosition();
    void UpdateAtmosphericProperties();
    ELight_TimeOfDay CalculateTimeOfDay(float TimeValue);
};

#include "Light_AtmosphereManager.generated.h"