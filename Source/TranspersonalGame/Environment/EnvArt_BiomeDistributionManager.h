#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "EnvArt_BiomeDistributionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius;

    FEnvArt_BiomeCoordinates()
        : BiomeName(TEXT("Unknown"))
        , CenterLocation(FVector::ZeroVector)
        , SpawnRadius(15000.0f)
    {}

    FEnvArt_BiomeCoordinates(const FString& InBiomeName, const FVector& InCenter, float InRadius = 15000.0f)
        : BiomeName(InBiomeName)
        , CenterLocation(InCenter)
        , SpawnRadius(InRadius)
    {}
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_BiomeDistributionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_BiomeDistributionManager();

protected:
    virtual void BeginPlay() override;

public:
    // Core biome distribution functionality
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeBiomeCoordinates();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    FVector GetRandomLocationInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    TArray<FVector> GetDistributedLocationsAcrossBiomes(int32 TotalCount) const;

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void DistributeActorsAcrossBiomes(const TArray<AActor*>& ActorsToDistribute);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    FString GetBiomeNameAtLocation(const FVector& Location) const;

    // Dinosaur-specific distribution
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void DistributeDinosaursAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SpawnDinosaurInBiome(const FString& DinosaurType, const FString& BiomeName, int32 Count = 1);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    TArray<FEnvArt_BiomeCoordinates> BiomeCoordinates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float DistributionPercentagePerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    bool bAutoDistributeOnBeginPlay;

private:
    void SetupDefaultBiomes();
    FVector GetRandomOffsetInRadius(const FVector& Center, float Radius) const;
    bool IsLocationInBiome(const FVector& Location, const FEnvArt_BiomeCoordinates& Biome) const;
};