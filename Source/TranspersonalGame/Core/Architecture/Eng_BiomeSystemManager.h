#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Eng_BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class APawn>> AllowedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaursPerBiome = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float SpawnRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<class UStaticMesh*> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    FEng_BiomeConfiguration()
    {
        BiomeType = EBiomeType::Forest;
        Temperature = 25.0f;
        Humidity = 60.0f;
        Elevation = 100.0f;
        MaxDinosaursPerBiome = 25;
        SpawnRadius = 5000.0f;
        VegetationDensity = 0.7f;
        AmbientColor = FLinearColor::White;
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Desert;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Forest;
        ToBiome = EBiomeType::Desert;
        TransitionDistance = 1000.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeConfiguration(EBiomeType BiomeType, const FEng_BiomeConfiguration& Configuration);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetAllBiomeTypes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationSuitableForDinosaur(const FVector& WorldLocation, TSubclassOf<APawn> DinosaurClass) const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeTransition GetBiomeTransition(EBiomeType FromBiome, EBiomeType ToBiome) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeBlendFactor(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const;

    // World Generation Integration
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateBiomeMap(int32 WorldSizeKm = 16);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ApplyBiomeToRegion(const FVector& CenterLocation, float Radius, EBiomeType BiomeType);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeCache();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ClearBiomeCache();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void DebugDrawBiomeBoundaries();

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void ValidateBiomeConfiguration();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 BiomeCacheResolution = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeUpdateFrequency = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableBiomeDebugDraw = false;

private:
    // Internal biome calculation
    EBiomeType CalculateBiomeFromEnvironment(const FVector& WorldLocation) const;
    void InitializeDefaultBiomeConfigurations();
    void CacheBiomeData();

    // Cached biome data for performance
    TMap<FIntPoint, EBiomeType> BiomeCache;
    FTimerHandle BiomeUpdateTimer;
};