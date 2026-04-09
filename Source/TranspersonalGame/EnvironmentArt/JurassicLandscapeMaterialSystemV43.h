#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeLayerInfoObject.h"
#include "JurassicLandscapeMaterialSystemV43.generated.h"

// Forward declarations
class UMaterialInterface;
class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class ALandscape;
class ULandscapeLayerInfoObject;
class UTexture2D;

UENUM(BlueprintType)
enum class EJurassicTerrainType : uint8
{
    JurassicSoil        UMETA(DisplayName = "Jurassic Soil"),
    VolcanicRock        UMETA(DisplayName = "Volcanic Rock"),
    RiverSediment       UMETA(DisplayName = "River Sediment"),
    ForestFloor         UMETA(DisplayName = "Forest Floor"),
    CliffStone          UMETA(DisplayName = "Cliff Stone"),
    SwampMud            UMETA(DisplayName = "Swamp Mud"),
    AncientSandstone    UMETA(DisplayName = "Ancient Sandstone"),
    FossilBed           UMETA(DisplayName = "Fossil Bed")
};

UENUM(BlueprintType)
enum class EJurassicWeatherState : uint8
{
    Clear               UMETA(DisplayName = "Clear"),
    LightRain           UMETA(DisplayName = "Light Rain"),
    HeavyRain           UMETA(DisplayName = "Heavy Rain"),
    Fog                 UMETA(DisplayName = "Fog"),
    Storm               UMETA(DisplayName = "Storm"),
    Humid               UMETA(DisplayName = "Humid")
};

UENUM(BlueprintType)
enum class EJurassicTimeOfDay : uint8
{
    Dawn                UMETA(DisplayName = "Dawn"),
    Morning             UMETA(DisplayName = "Morning"),
    Midday              UMETA(DisplayName = "Midday"),
    Afternoon           UMETA(DisplayName = "Afternoon"),
    Dusk                UMETA(DisplayName = "Dusk"),
    Night               UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicTerrainMaterialSpec
{
    GENERATED_BODY()

    // Basic Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    EJurassicTerrainType TerrainType = EJurassicTerrainType::JurassicSoil;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TSoftObjectPtr<UMaterialInterface> BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material")
    TSoftObjectPtr<ULandscapeLayerInfoObject> LayerInfo;

    // Physically Based Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    FLinearColor BaseColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Roughness = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Metallic = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PBR", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Specular = 0.5f;

    // Texture Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> DiffuseTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> NormalTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> RoughnessTexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Textures")
    TSoftObjectPtr<UTexture2D> HeightTexture;

    // UV and Tiling
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV", meta = (ClampMin = "0.01", ClampMax = "10.0"))
    float TilingScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV", meta = (ClampMin = "-360.0", ClampMax = "360.0"))
    float TilingRotation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UV")
    FVector2D TilingOffset = FVector2D::ZeroVector;

    // Blending Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending")
    bool bUseHeightBlending = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float BlendSharpness = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blending", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeightBlendBias = 0.0f;

    // Environmental Response
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float WetnessResponse = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float TemperatureResponse = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FLinearColor WetColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);

    // Storytelling Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bCanShowFossils = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bCanShowWeathering = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AgeVariation = 0.3f;

    FJurassicTerrainMaterialSpec()
    {
        // Default values for Jurassic soil
        BaseColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
        Roughness = 0.8f;
        Metallic = 0.0f;
        Specular = 0.5f;
        TilingScale = 1.0f;
        WetnessResponse = 1.0f;
        TemperatureResponse = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FJurassicEnvironmentalState
{
    GENERATED_BODY()

    // Weather and Climate
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    EJurassicWeatherState CurrentWeather = EJurassicWeatherState::Clear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatherIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Humidity = 0.7f; // Jurassic was generally humid

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather", meta = (ClampMin = "15.0", ClampMax = "35.0"))
    float Temperature = 25.0f; // Celsius, Jurassic was warm

    // Time and Lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
    EJurassicTimeOfDay TimeOfDay = EJurassicTimeOfDay::Midday;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time", meta = (ClampMin = "0.0", ClampMax = "24.0"))
    float TimeOfDayFloat = 12.0f; // 24-hour format

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float SunIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);

    // Surface Conditions
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SurfaceWetness = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SurfaceTemperature = 0.5f; // Normalized temperature

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Surface", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WindStrength = 0.3f;

    FJurassicEnvironmentalState()
    {
        // Default Jurassic environmental conditions
        CurrentWeather = EJurassicWeatherState::Clear;
        WeatherIntensity = 0.0f;
        Humidity = 0.7f;
        Temperature = 25.0f;
        TimeOfDay = EJurassicTimeOfDay::Midday;
        TimeOfDayFloat = 12.0f;
        SunIntensity = 1.0f;
        SunColor = FLinearColor(1.0f, 0.95f, 0.8f, 1.0f);
        SurfaceWetness = 0.0f;
        SurfaceTemperature = 0.5f;
        WindStrength = 0.3f;
    }
};

/**
 * Data Asset containing all Jurassic terrain material specifications
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UJurassicLandscapeMaterialDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Materials")
    TArray<FJurassicTerrainMaterialSpec> TerrainMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Materials")
    TSoftObjectPtr<UMaterialInterface> MasterLandscapeMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Master Materials")
    TSoftObjectPtr<UMaterialParameterCollection> EnvironmentalParameterCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalTilingScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableWeatherEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bEnableTimeOfDayEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseNaniteLandscape = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableVirtualTexturing = true;

    // Helper functions
    UFUNCTION(BlueprintCallable, Category = "Material System")
    FJurassicTerrainMaterialSpec GetTerrainMaterialSpec(EJurassicTerrainType TerrainType) const;

    UFUNCTION(BlueprintCallable, Category = "Material System")
    TArray<FJurassicTerrainMaterialSpec> GetAllTerrainMaterials() const { return TerrainMaterials; }

    UFUNCTION(BlueprintCallable, Category = "Material System")
    bool HasTerrainMaterial(EJurassicTerrainType TerrainType) const;
};

/**
 * Main Jurassic Landscape Material System Component
 * Manages dynamic landscape materials and environmental responses
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UJurassicLandscapeMaterialSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UJurassicLandscapeMaterialSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material System")
    TSoftObjectPtr<UJurassicLandscapeMaterialDataAsset> MaterialDataAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material System")
    TSoftObjectPtr<ALandscape> TargetLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material System")
    bool bAutoApplyOnBeginPlay = true;

    // Environmental State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    FJurassicEnvironmentalState CurrentEnvironmentalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableDynamicWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bEnableTimeOfDayEffects = true;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseSubstrateMaterials = false; // Experimental

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bOptimizeForMobile = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "2.0"))
    float MaterialLODBias = 1.0f;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<UMaterialInstanceDynamic*> DynamicMaterialInstances;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bMaterialSystemInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float LastEnvironmentalUpdate = 0.0f;

    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Material System")
    void InitializeMaterialSystem();

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void ApplyMaterialsToLandscape();

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void UpdateEnvironmentalState(const FJurassicEnvironmentalState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void SetWeatherState(EJurassicWeatherState NewWeather, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void SetTimeOfDay(EJurassicTimeOfDay NewTimeOfDay, float TimeFloat);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void UpdateSurfaceWetness(float WetnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Material System")
    void UpdateSurfaceTemperature(float TemperatureLevel);

    // Material Blending
    UFUNCTION(BlueprintCallable, Category = "Material Blending")
    void SetTerrainLayerWeight(EJurassicTerrainType TerrainType, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Material Blending")
    float GetTerrainLayerWeight(EJurassicTerrainType TerrainType) const;

    UFUNCTION(BlueprintCallable, Category = "Material Blending")
    void BlendTerrainMaterials(const TMap<EJurassicTerrainType, float>& MaterialWeights);

    // Storytelling Features
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddFossilDecal(FVector Location, float Size, EJurassicTerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void AddWeatheringEffect(FVector Location, float Radius, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAncientRiverbed(const TArray<FVector>& PathPoints, float Width);

    // Performance and Quality
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaterialQuality(int32 QualityLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForCurrentHardware();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODSettings(float NewLODBias);

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugShowMaterialLayers();

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
    void DebugShowEnvironmentalState();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void VisualizeTerrainBlending(bool bEnable);

private:
    // Internal Systems
    void SetupLandscapeReference();
    void CreateDynamicMaterialInstances();
    void UpdateMaterialParameters();
    void ProcessEnvironmentalEffects(float DeltaTime);
    void UpdateWeatherEffects();
    void UpdateTimeOfDayEffects();
    
    // Material Helpers
    UMaterialInstanceDynamic* GetOrCreateDynamicMaterial(EJurassicTerrainType TerrainType);
    void SetMaterialParameter(const FString& ParameterName, float Value);
    void SetMaterialParameter(const FString& ParameterName, const FLinearColor& Value);
    void SetMaterialParameter(const FString& ParameterName, UTexture* Texture);
    
    // Environmental Processing
    void CalculateWeatherEffects();
    void CalculateLightingEffects();
    void UpdateSurfaceProperties();
    
    // Performance Monitoring
    void MonitorMaterialPerformance();
    void AdjustMaterialQuality();
    
    // Cache
    UPROPERTY()
    TMap<EJurassicTerrainType, FJurassicTerrainMaterialSpec> CachedMaterialSpecs;
    
    UPROPERTY()
    TMap<EJurassicTerrainType, UMaterialInstanceDynamic*> MaterialInstanceCache;
    
    // Runtime Data
    float EnvironmentalUpdateInterval = 0.1f; // 10 FPS for environmental updates
    bool bPerformanceOptimizationActive = false;
    int32 CurrentMaterialQuality = 3; // 0-4 scale
};