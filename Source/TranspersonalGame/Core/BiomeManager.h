#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurs;

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Savanna;
        Center = FVector::ZeroVector;
        Radius = 10000.0f;
        Temperature = 20.0f;
        Humidity = 50.0f;
    }
};

/**
 * BiomeManager - Core system for managing the 5 biomes in the prehistoric world
 * Handles biome boundaries, environmental effects, and spawn location validation
 * Critical for World Generation and Environment Art systems
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core biome data for all 5 biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FEng_BiomeData> BiomeData;

    // Biome management functions
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetHumidityAtLocation(const FVector& Location) const;

    // Validation functions for other systems
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsValidSpawnLocation(const FVector& Location, EBiomeType RequiredBiome) const;

private:
    // Initialize the 5 biomes with correct coordinates
    void InitializeBiomeData();
    
    // Setup biome boundary systems
    void SetupBiomeBoundaries();
    
    // Update environmental effects
    void UpdateBiomeEffects(float DeltaTime);
};