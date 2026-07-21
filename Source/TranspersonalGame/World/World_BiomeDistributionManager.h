#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "World_BiomeDistributionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeSpawnData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float SpawnDensity;

    FWorld_BiomeSpawnData()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        SpawnRadius = 15000.0f;
        MaxActorsPerBiome = 1000;
        SpawnDensity = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeDistributionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeDistributionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    TArray<FWorld_BiomeSpawnData> BiomeSpawnAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    bool bAutoDistributeOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Distribution")
    float MinDistanceBetweenActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void InitializeBiomeAreas();

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void DistributeActorsAcrossBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    FString GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    int32 GetActorCountInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void SpawnActorInBiome(const FString& BiomeName, UClass* ActorClass, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution")
    void RebalanceBiomeDistribution();

    UFUNCTION(BlueprintCallable, Category = "Biome Distribution", CallInEditor = true)
    void DebugPrintBiomeStats();

private:
    void SetupDefaultBiomes();
    bool IsLocationValidForSpawn(const FVector& Location, const FString& BiomeName);
    TArray<AActor*> GetActorsInBiome(const FString& BiomeName);
};