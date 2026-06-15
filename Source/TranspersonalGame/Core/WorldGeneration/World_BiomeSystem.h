#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest          UMETA(DisplayName = "Dense Forest"),
    Plains          UMETA(DisplayName = "Open Plains"),
    RiverValley     UMETA(DisplayName = "River Valley"),
    RockyOutcrop    UMETA(DisplayName = "Rocky Outcrop"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    VolcanicRegion  UMETA(DisplayName = "Volcanic Region")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BiomeCenter = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterCoverage = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeData()
    {
        VegetationTypes.Add("Fern");
        VegetationTypes.Add("Conifer");
        DinosaurSpecies.Add("Triceratops");
        DinosaurSpecies.Add("Parasaurolophus");
    }
};

UCLASS(ClassGroup=(WorldGeneration), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FWorld_BiomeData> ActiveBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 MaxBiomes = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float BiomeBlendDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    bool bAutoGenerateBiomes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    FVector WorldBounds = FVector(20000.0f, 20000.0f, 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateRandomBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void AddBiome(const FWorld_BiomeData& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FVector> GetWaterSourceLocations() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FString> GetDinosaurSpeciesAtLocation(const FVector& WorldLocation) const;

protected:
    UFUNCTION()
    void InitializeDefaultBiomes();

    UFUNCTION()
    float CalculateBiomeInfluence(const FVector& WorldLocation, const FWorld_BiomeData& Biome) const;

    UPROPERTY()
    bool bIsInitialized = false;
};