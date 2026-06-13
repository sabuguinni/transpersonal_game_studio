#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EVegetationType> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaursPerBiome = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ResourceAbundance = 1.0f;

    FEng_BiomeConfiguration()
    {
        NativeDinosaurs.Add(EDinosaurSpecies::Parasaurolophus);
        VegetationTypes.Add(EVegetationType::Fern);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Desert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_BiomeSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Configuration
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeConfiguration(EBiomeType BiomeType, const FEng_BiomeConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetAvailableBiomes() const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeBlendFactor(const FVector& WorldLocation, EBiomeType& PrimaryBiome, EBiomeType& SecondaryBiome) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RemoveBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome);

    // Environmental Queries
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EDinosaurSpecies> GetNativeDinosaursAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EVegetationType> GetVegetationTypesAtLocation(const FVector& WorldLocation) const;

    // Spawning Support
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanSpawnDinosaurAtLocation(const FVector& WorldLocation, EDinosaurSpecies Species) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    int32 GetMaxDinosaursForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetSpawnRateMultiplierAtLocation(const FVector& WorldLocation) const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void DebugDrawBiomeBoundaries();

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void LogBiomeSystemStatus() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float BiomeGridSize = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    int32 BiomeNoiseOctaves = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    float BiomeNoiseScale = 0.001f;

private:
    void SetupDefaultBiomeConfigurations();
    float GenerateBiomeNoise(const FVector& Location, int32 Seed) const;
    EBiomeType DetermineBiomeFromNoise(float NoiseValue, float Elevation) const;
};

#include "Eng_BiomeSystemManager.generated.h"