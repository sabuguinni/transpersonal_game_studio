#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Light_ArchitecturalIntegrationManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ArchitecturalLightingConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    float DirectionalLightIntensity = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    FLinearColor DirectionalLightColor = FLinearColor(1.0f, 0.96f, 0.82f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    float DirectionalLightTemperature = 5500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    float PointLightIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    float PointLightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architectural Lighting")
    FLinearColor PointLightColor = FLinearColor(1.0f, 0.86f, 0.71f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Effects")
    FLinearColor FogInscatteringColor = FLinearColor(0.71f, 0.78f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Illumination")
    float SkyLightIntensity = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Illumination")
    FLinearColor SkyLightColor = FLinearColor(0.78f, 0.86f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLumenGI = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLumenReflections = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bCastVolumetricShadows = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_ArchitecturalLightingMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePointLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 ShadowCastingLights = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float LumenGIPerformanceCost = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float VolumetricFogPerformanceCost = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    bool bAtmospherePersistenceActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float LastOptimizationTime = 0.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_ArchitecturalIntegrationManager : public AActor
{
    GENERATED_BODY()

public:
    ALight_ArchitecturalIntegrationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FLight_ArchitecturalLightingConfig LightingConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Metrics")
    FLight_ArchitecturalLightingMetrics PerformanceMetrics;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    // Light component references
    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ADirectionalLight* MainDirectionalLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class ASkyLight* GlobalSkyLight;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    class AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "Light References")
    TArray<class APointLight*> ArchitecturalPointLights;

public:
    // Core lighting management functions
    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void InitializeArchitecturalLighting();

    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void ApplyLightingConfiguration(const FLight_ArchitecturalLightingConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Architectural Lighting")
    void CreateArchitecturalPointLight(const FVector& Location, const FString& LightName);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void ConfigureAtmosphericEffects();

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Effects")
    void FixPersistentAtmosphere();

    UFUNCTION(BlueprintCallable, Category = "Global Illumination")
    void EnableLumenGlobalIllumination();

    UFUNCTION(BlueprintCallable, Category = "Global Illumination")
    void ConfigureSkyLighting();

    // Performance optimization functions
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLightingPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODBasedLighting(float DistanceFromPlayer);

    // Integration with architectural systems
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithStructuralOptimization(class AActor* StructuralActor);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void UpdateLightingForWeatheringState(float WeatheringLevel);

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void SynchronizeWithBiomeLighting(const FVector& BiomeLocation);

    // Persistence and save functions
    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void SaveLightingConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    void LoadLightingConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Persistence")
    bool ValidateAtmospherePersistence();

    // Debug and validation functions
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugLightingSetup();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateLightingIntegration();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetLightingStatusReport();

protected:
    // Internal helper functions
    void FindExistingLightActors();
    void CleanupApocalypticAtmosphere();
    void SetupVolumetricEffects();
    void ConfigureConsoleVariables();
    void UpdateLightingMetrics();

    // Performance tracking
    float LastPerformanceCheck = 0.0f;
    float PerformanceCheckInterval = 5.0f;
    
    // Persistence tracking
    bool bAtmosphereConfigured = false;
    float LastAtmosphereCheck = 0.0f;
    float AtmosphereCheckInterval = 30.0f;
};