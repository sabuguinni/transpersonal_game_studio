#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "EnvironmentManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EEnvArt_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Morning     UMETA(DisplayName = "Morning"),
    Noon        UMETA(DisplayName = "Noon"),
    Afternoon   UMETA(DisplayName = "Afternoon"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

USTRUCT(BlueprintType)
struct FEnvArt_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType BiomeType = EEnvArt_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    int32 MaxPropsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    float PropDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor AmbientColor = FLinearColor(0.3f, 0.3f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.7f, 0.8f, 1.0f);
};

USTRUCT(BlueprintType)
struct FEnvArt_LightingSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphericDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor ScatteringColor = FLinearColor(0.9f, 0.7f, 0.5f, 1.0f);
};

/**
 * Environment Manager - Handles atmospheric lighting, biome-specific props, and visual storytelling
 * Responsible for creating immersive prehistoric environments that feel lived-in and authentic
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvironmentManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvironmentManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Environment Management
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void InitializeEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetupBiomes();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiomeWithProps(EEnvArt_BiomeType BiomeType, int32 PropCount);

    // Lighting and Atmosphere
    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetTimeOfDay(EEnvArt_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void UpdateSunPosition(float TimeOfDayNormalized);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void ConfigureAtmosphericLighting(const FEnvArt_LightingSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Lighting")
    void SetupVolumetricFog();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEnvArt_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEnvArt_BiomeSettings GetBiomeSettings(EEnvArt_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeSettings(EEnvArt_BiomeType BiomeType, const FEnvArt_BiomeSettings& NewSettings);

    // Prop Management
    UFUNCTION(BlueprintCallable, Category = "Props")
    void SpawnEnvironmentProp(const FVector& Location, EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Props")
    void CleanupExcessProps();

    UFUNCTION(BlueprintCallable, Category = "Props")
    int32 GetTotalActorCount() const;

    // Visual Storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateNarrativeCluster(const FVector& Location, const FString& StoryTheme);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceAbandonedCampsite(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateDinosaurNest(const FVector& Location, const FString& DinosaurType);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    TMap<EEnvArt_BiomeType, FEnvArt_BiomeSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    FEnvArt_LightingSettings CurrentLightingSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    EEnvArt_TimeOfDay CurrentTimeOfDay = EEnvArt_TimeOfDay::Afternoon;

    // Actor References
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    AExponentialHeightFog* AtmosphericFog;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    TArray<AActor*> SpawnedProps;

    // Limits and Constraints
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxTotalActors = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxPropsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
    int32 MaxTotalProps = 5000;

private:
    // Internal helper functions
    void InitializeBiomeDefaults();
    FVector GetRandomLocationInBiome(EEnvArt_BiomeType BiomeType) const;
    void ApplyBiomeSpecificMaterials(AActor* PropActor, EEnvArt_BiomeType BiomeType);
    bool IsLocationSuitableForProp(const FVector& Location) const;
    void RemoveOldestProps(int32 CountToRemove);
};