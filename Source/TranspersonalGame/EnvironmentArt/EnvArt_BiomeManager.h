#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Texture2D.h"
#include "../SharedTypes.h"
#include "EnvArt_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Primordial Forest"),
    Rocky           UMETA(DisplayName = "Rocky Outcrops"),
    Wetland         UMETA(DisplayName = "Prehistoric Wetlands"),
    Volcanic        UMETA(DisplayName = "Volcanic Terrain"),
    Plains          UMETA(DisplayName = "Open Plains"),
    Desert          UMETA(DisplayName = "Arid Desert"),
    Coastal         UMETA(DisplayName = "Coastal Areas"),
    Cave            UMETA(DisplayName = "Cave Systems")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType BiomeType = EEnvArt_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> GroundMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> RockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Props")
    TArray<TSoftObjectPtr<UStaticMesh>> EnvironmentProps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Density")
    float PropDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FLinearColor AmbientTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float FogDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);

    FEnvArt_BiomeSettings()
    {
        VegetationDensity = 1.0f;
        PropDensity = 0.5f;
        AmbientTint = FLinearColor::White;
        FogDensity = 0.1f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropPlacement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FRotator Rotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector Scale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    float SpawnProbability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bRandomRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    FVector2D ScaleRange = FVector2D(0.8f, 1.2f);

    FEnvArt_PropPlacement()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        SpawnProbability = 1.0f;
        bAlignToSurface = true;
        bRandomRotation = true;
        ScaleRange = FVector2D(0.8f, 1.2f);
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_BiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Settings")
    TMap<EEnvArt_BiomeType, FEnvArt_BiomeSettings> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current Biome")
    EEnvArt_BiomeType CurrentBiome = EEnvArt_BiomeType::Forest;

    // Environment Population
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    TArray<FEnvArt_PropPlacement> ActivePropPlacements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    float PopulationRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 MaxPropsPerBiome = 1000;

    // Material Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TMap<EEnvArt_BiomeType, TSoftObjectPtr<UMaterialInterface>> BiomeMaterials;

    // Narrative Elements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableEnvironmentalStorytelling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingDensity = 0.1f;

    // Core Functions
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void SetCurrentBiome(EEnvArt_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FEnvArt_BiomeSettings GetBiomeSettings(EEnvArt_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Population")
    void PopulateEnvironment(const FVector& CenterLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Environment Population")
    void ClearEnvironmentProps();

    UFUNCTION(BlueprintCallable, Category = "Environment Population")
    void PlaceProp(const FEnvArt_PropPlacement& PropPlacement);

    UFUNCTION(BlueprintCallable, Category = "Material Management")
    void ApplyBiomeMaterials(EEnvArt_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingElements(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EEnvArt_BiomeType DetectBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Population")
    bool ValidatePropPlacement(const FVector& Location, float MinDistance = 100.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Population")
    FVector GetSurfaceLocation(const FVector& Location) const;

private:
    // Internal state
    TArray<AActor*> SpawnedProps;
    float LastPopulationUpdate = 0.0f;
    FVector LastPlayerLocation = FVector::ZeroVector;

    // Helper functions
    void InitializeBiomeConfigurations();
    void UpdateEnvironmentLOD(const FVector& ViewerLocation);
    void CreateStorytellingCluster(const FVector& Location, int32 ClusterType);
    bool IsLocationSuitable(const FVector& Location, EEnvArt_BiomeType BiomeType) const;
    FRotator CalculateAlignedRotation(const FVector& Location) const;
};