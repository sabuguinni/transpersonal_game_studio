#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActorsPerBiome;

    FWorld_BiomeConfig()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        VegetationDensity = 0.5f;
        RockDensity = 0.3f;
        MaxActorsPerBiome = 1000;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void PopulateBiome(const FString& BiomeName, int32 VegetationCount, int32 RockCount);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetActorCountInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CleanupExcessActors();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FString> GetAllBiomeNames();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY()
    TMap<FString, TArray<AActor*>> BiomeActors;

private:
    void SetupDefaultBiomes();
    AActor* SpawnVegetationActor(const FVector& Location);
    AActor* SpawnRockActor(const FVector& Location);
    bool IsLocationInBiome(const FVector& Location, const FWorld_BiomeConfig& Biome);
};