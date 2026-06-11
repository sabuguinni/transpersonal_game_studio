#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

/**
 * ENGINE ARCHITECT BIOME SYSTEM - CORE ARCHITECTURE
 * Manages biome transitions, environmental parameters, and ecosystem rules
 * Integrates with PCGWorldGenerator for procedural biome placement
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeInfluenceRadius = 2000.0f;

    FEng_BiomeParameters()
    {
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
        WaterDistance = 1000.0f;
        BiomeType = EBiomeType::Grassland;
        BiomeInfluenceRadius = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;

    FEng_BiomeTransition()
    {
        FromBiome = EBiomeType::Grassland;
        ToBiome = EBiomeType::Forest;
        TransitionDistance = 500.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // CORE COMPONENTS
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VisualizationMesh;

    // BIOME SYSTEM PARAMETERS
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeParameters> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float GlobalTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float GlobalHumidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float SeasonalVariation = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeUpdateInterval = 5.0f;

    // ECOSYSTEM RULES
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableDynamicWeather = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableSeasonalChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    bool bEnableBiomeTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float EcosystemComplexity = 1.0f;

public:
    // BIOME QUERY FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeParameters GetBiomeParameters(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiomeTransition(FVector Location) const;

    // BIOME MANAGEMENT FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeRegion(const FEng_BiomeParameters& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RemoveBiomeRegion(int32 RegionIndex);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RefreshAllBiomes();

    // INTEGRATION FUNCTIONS
    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithWorldGenerator();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void IntegrateWithFoliageSystem();

    UFUNCTION(BlueprintCallable, Category = "Integration")
    void NotifyEnvironmentChange(FVector Location, float Radius);

private:
    // INTERNAL STATE
    float LastUpdateTime;
    bool bIsInitialized;
    
    // HELPER FUNCTIONS
    float CalculateBiomeInfluence(const FEng_BiomeParameters& Biome, FVector Location) const;
    FEng_BiomeParameters BlendBiomeParameters(const FEng_BiomeParameters& BiomeA, const FEng_BiomeParameters& BiomeB, float BlendFactor) const;
    void InitializeBiomeSystem();
    void UpdateVisualization();
};