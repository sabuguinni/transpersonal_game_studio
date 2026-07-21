#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomeSystemManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Mountains   UMETA(DisplayName = "Mountains")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeData()
    {
        DinosaurSpecies.Add("TRex");
        DinosaurSpecies.Add("Velociraptor");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<AStaticMeshActor*> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<AStaticMeshActor*> VegetationActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateWaterSystems();

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void PopulateVegetation();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnBiomeSpecificDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeData& BiomeData) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawBiomeBoundaries();

protected:
    void SetupDefaultBiomes();
    void CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation);
    void SpawnVegetationInBiome(const FWorld_BiomeData& BiomeData);
    void SpawnDinosaursInBiome(const FWorld_BiomeData& BiomeData);
};