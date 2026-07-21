#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentalAssets.generated.h"

/**
 * Environmental Asset Types for Cretaceous Period
 * Defines categories and properties for prehistoric environmental props
 */
UENUM(BlueprintType)
enum class EEnvArt_CretaceousAssetType : uint8
{
    FallenLog           UMETA(DisplayName = "Fallen Log"),
    AncientRock         UMETA(DisplayName = "Ancient Rock Formation"),
    FernCluster         UMETA(DisplayName = "Fern Cluster"),
    BoulderFormation    UMETA(DisplayName = "Boulder Formation"),
    FossilizedBones     UMETA(DisplayName = "Fossilized Bones"),
    PrehistoricNest     UMETA(DisplayName = "Prehistoric Nest"),
    WeatheredStone      UMETA(DisplayName = "Weathered Stone"),
    MossyCoveredRock    UMETA(DisplayName = "Mossy Covered Rock")
};

/**
 * Environmental Asset Spawn Configuration
 * Controls how and where environmental assets are placed
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AssetSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    EEnvArt_CretaceousAssetType AssetType = EEnvArt_CretaceousAssetType::FallenLog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    FRotator SpawnRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    FVector ScaleRange = FVector(0.8f, 1.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset Config")
    bool bGroundAlign = true;
};

/**
 * Atmospheric Enhancement Configuration
 * Controls lighting, fog, and particle effects for environmental atmosphere
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericConfig
{
    GENERATED_BODY()

    // Lighting Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.86f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunTemperature = 3200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    // Fog Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogMaxOpacity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogStartDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogCutoffDistance = 8000.0f;

    // Volumetric Fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricScatteringDistribution = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    FColor VolumetricAlbedo = FColor(240, 240, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volumetric Fog")
    float VolumetricExtinctionScale = 1.0f;
};

/**
 * Environmental Storytelling Element Configuration
 * Defines narrative props that tell environmental stories
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_StorytellingElement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString ElementName = TEXT("Unnamed Story Element");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FString StoryDescription = TEXT("Environmental story element");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<EEnvArt_CretaceousAssetType> RequiredAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float InteractionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bPlayerDiscoverable = true;
};

/**
 * Cretaceous Environmental Assets Manager
 * Manages placement, configuration, and optimization of environmental assets
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousEnvironmentalAssets : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousEnvironmentalAssets();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Asset Management
    UFUNCTION(BlueprintCallable, Category = "Environmental Assets")
    void SpawnEnvironmentalAsset(const FEnvArt_AssetSpawnConfig& SpawnConfig);

    UFUNCTION(BlueprintCallable, Category = "Environmental Assets")
    void SpawnAssetCluster(EEnvArt_CretaceousAssetType AssetType, const FVector& CenterLocation, int32 Count, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environmental Assets")
    void ClearAllEnvironmentalAssets();

    // Atmospheric Control
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericSettings(const FEnvArt_AtmosphericConfig& AtmosphericConfig);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void EnableVolumetricFog(bool bEnable);

    // Environmental Storytelling
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateStorytellingScene(const FEnvArt_StorytellingElement& StoryElement);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    TArray<FEnvArt_StorytellingElement> GetNearbyStoryElements(const FVector& PlayerLocation, float SearchRadius);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeEnvironmentalAssets();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistances(float Near, float Medium, float Far);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FEnvArt_AtmosphericConfig DefaultAtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEnvArt_AssetSpawnConfig> DefaultAssetSpawns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEnvArt_StorytellingElement> StorytellingElements;

    // Runtime Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedAssets;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    int32 TotalAssetsSpawned = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float LastOptimizationTime = 0.0f;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxAssetsPerFrame = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float OptimizationInterval = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance = 5000.0f;

private:
    // Internal Methods
    AActor* SpawnAssetActor(EEnvArt_CretaceousAssetType AssetType, const FVector& Location, const FRotator& Rotation, const FVector& Scale);
    void ApplyAssetMaterial(AActor* Asset, EEnvArt_CretaceousAssetType AssetType);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);
    bool IsLocationValid(const FVector& Location);
    void UpdateAssetLOD(AActor* Asset, float DistanceToPlayer);
};