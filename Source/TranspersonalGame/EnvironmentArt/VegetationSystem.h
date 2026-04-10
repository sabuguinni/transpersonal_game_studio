#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "VegetationSystem.generated.h"

UENUM(BlueprintType)
enum class EVegetationType : uint8
{
    Grass           UMETA(DisplayName = "Grass"),
    Ferns           UMETA(DisplayName = "Ferns"), 
    Trees           UMETA(DisplayName = "Trees"),
    Bushes          UMETA(DisplayName = "Bushes"),
    Flowers         UMETA(DisplayName = "Flowers"),
    Moss            UMETA(DisplayName = "Moss"),
    Vines           UMETA(DisplayName = "Vines")
};

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Grassland       UMETA(DisplayName = "Grassland"),
    Swampland       UMETA(DisplayName = "Swampland"),
    Rocky           UMETA(DisplayName = "Rocky"),
    Riverbank       UMETA(DisplayName = "Riverbank"),
    Hills           UMETA(DisplayName = "Hills")
};

USTRUCT(BlueprintType)
struct FVegetationData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EVegetationType VegetationType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float WindInfluence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<EBiomeType> PreferredBiomes;

    FVegetationData()
    {
        VegetationType = EVegetationType::Grass;
        MinScale = FVector(0.8f, 0.8f, 0.8f);
        MaxScale = FVector(1.2f, 1.2f, 1.2f);
        Density = 1.0f;
        WindInfluence = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FBiomeVegetationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FVegetationData> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float OverallDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiodiversityFactor;

    FBiomeVegetationSettings()
    {
        BiomeType = EBiomeType::Forest;
        OverallDensity = 1.0f;
        BiodiversityFactor = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UVegetationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UVegetationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core vegetation functions
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void GenerateVegetationForBiome(EBiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void ClearVegetationInArea(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void UpdateVegetationLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SetWindParameters(const FVector& WindDirection, float WindStrength);

    // Biome configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FBiomeVegetationSettings> BiomeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    UDataTable* VegetationDataTable;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxInstancesPerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistanceMultiplier;

    // Wind system
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    FVector WindDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wind")
    float WindVariation;

private:
    // Internal vegetation management
    UPROPERTY()
    TMap<EVegetationType, UInstancedStaticMeshComponent*> VegetationInstances;

    UPROPERTY()
    TArray<FVector> GeneratedPositions;

    // Helper functions
    bool CanPlaceVegetationAt(const FVector& Location, EVegetationType Type);
    FVector GetRandomScale(const FVegetationData& VegData);
    FRotator GetRandomRotation(EVegetationType Type);
    float CalculateDensityAtLocation(const FVector& Location, EBiomeType BiomeType);
    
    // Performance optimization
    void OptimizeInstancedMeshes();
    void UpdateCullingDistances();
};