#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousPropSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousPropData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FString PropName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    TSoftObjectPtr<UStaticMesh> PropMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    FVector MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float SpawnProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    int32 MaxInstancesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    float MinDistanceBetween;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bAllowRotationVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Data")
    bool bSnapToGround;

    FEnvArt_CretaceousPropData()
    {
        PropName = TEXT("DefaultProp");
        MinScale = FVector(0.8f, 0.8f, 0.8f);
        MaxScale = FVector(1.2f, 1.2f, 1.2f);
        SpawnProbability = 0.5f;
        MaxInstancesPerBiome = 50;
        MinDistanceBetween = 500.0f;
        bAllowRotationVariation = true;
        bSnapToGround = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_PropClusterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    FVector ClusterCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    float ClusterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    TArray<FString> PropTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    int32 PropsInCluster;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cluster")
    EBiomeType BiomeType;

    FEnvArt_PropClusterData()
    {
        ClusterCenter = FVector::ZeroVector;
        ClusterRadius = 1000.0f;
        PropsInCluster = 0;
        BiomeType = EBiomeType::Forest;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousPropSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousPropSystem();

protected:
    virtual void BeginPlay() override;

    // Core prop data for different Cretaceous period props
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cretaceous Props")
    TMap<FString, FEnvArt_CretaceousPropData> CretaceousProps;

    // Active prop clusters in the world
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop Clusters")
    TArray<FEnvArt_PropClusterData> ActiveClusters;

    // Spawned prop actors for tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedProps;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPropsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PropCullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem;

public:
    // Initialize the prop system with Cretaceous period data
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    void InitializeCretaceousProps();

    // Create a prop cluster in a specific biome
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    bool CreatePropCluster(const FVector& Location, float Radius, EBiomeType BiomeType, int32 PropCount);

    // Spawn individual props based on environmental rules
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    AActor* SpawnEnvironmentalProp(const FString& PropType, const FVector& Location, const FRotator& Rotation);

    // Remove props that exceed performance limits
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    void OptimizePropDistribution();

    // Get props suitable for a specific biome
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    TArray<FString> GetPropsForBiome(EBiomeType BiomeType) const;

    // Check if location is valid for prop placement
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    bool IsValidPropLocation(const FVector& Location, const FString& PropType) const;

    // Update prop visibility based on distance
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    void UpdatePropLOD(const FVector& PlayerLocation);

    // Get total prop count for performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    int32 GetTotalPropCount() const;

    // Clear all props from a specific biome
    UFUNCTION(BlueprintCallable, Category = "Prop System")
    void ClearBiomeProps(EBiomeType BiomeType);

    // Environmental storytelling through prop placement
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateStorytellingScene(const FVector& Location, const FString& SceneType);

private:
    // Setup default Cretaceous prop data
    void SetupDefaultCretaceousProps();

    // Validate prop placement against terrain
    bool ValidateTerrainPlacement(const FVector& Location) const;

    // Calculate prop scale variation
    FVector CalculateScaleVariation(const FEnvArt_CretaceousPropData& PropData) const;

    // Get random rotation for prop variation
    FRotator GetRandomRotation(bool bAllowVariation) const;
};