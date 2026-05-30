#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/LightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "Light_AtmosphereManager.generated.h"

UENUM(BlueprintType)
enum class ELight_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"), 
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    ELight_BiomeType BiomeType = ELight_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FVector BiomeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float LightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float LightTemperature = 6500.0f;

    FLight_BiomeConfig()
    {
        BiomeType = ELight_BiomeType::Savana;
        BiomeCenter = FVector::ZeroVector;
        AmbientColor = FLinearColor::White;
        LightIntensity = 1.0f;
        LightTemperature = 6500.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_TimeOfDay
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float CurrentHour = 14.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunIntensity = 8.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
    FRotator SunRotation = FRotator(-25.0f, 45.0f, 0.0f);

    FLight_TimeOfDay()
    {
        CurrentHour = 14.0f;
        SunIntensity = 8.5f;
        SunTemperature = 3200.0f;
        SunRotation = FRotator(-25.0f, 45.0f, 0.0f);
    }
};

/**
 * Manages atmospheric lighting and biome-specific illumination for the Cretaceous period setting
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ULight_AtmosphereManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    ULight_AtmosphereManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetupCretaceousAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureBiomeSpecificLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void SetTimeOfDay(float Hour);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void ConfigureVolumetricFog();

    UFUNCTION(BlueprintPure, Category = "Atmospheric Lighting")
    FLight_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Lighting")
    void CreateBiomeAmbientLight(ELight_BiomeType BiomeType, FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Config")
    FLight_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Config")
    TArray<FLight_BiomeConfig> BiomeConfigs;

    UPROPERTY(BlueprintReadOnly, Category = "Atmospheric References")
    TWeakObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "Atmospheric References") 
    TWeakObjectPtr<ASkyAtmosphere> SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Config")
    bool bEnableDynamicTimeOfDay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Config")
    float TimeProgressionSpeed = 1.0f;

private:
    void InitializeBiomeConfigs();
    void FindAtmosphericActors();
    FLight_BiomeConfig GetBiomeConfig(ELight_BiomeType BiomeType) const;
};