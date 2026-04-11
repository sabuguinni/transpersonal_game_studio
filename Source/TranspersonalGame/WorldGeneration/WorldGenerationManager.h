#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/SceneComponent.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "WorldGenerationManager.generated.h"

class UBiomeDefinition;
class APCGVolume;
class UPCGComponent;

UENUM(BlueprintType)
enum class EBiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Mountain    UMETA(DisplayName = "Mountain"),
    River       UMETA(DisplayName = "River"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Coastal     UMETA(DisplayName = "Coastal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Size = FVector(2000.0f, 2000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UPCGGraph> PCGGraph;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = true;
};

/**
 * Central manager for procedural world generation using PCG and World Partition
 * Coordinates biome generation, terrain modification, and environmental systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorldGenerationManager : public AActor
{
    GENERATED_BODY()

public:
    AWorldGenerationManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    virtual void Tick(float DeltaTime) override;

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeWorldGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(EBiomeType BiomeType, const FVector& Location, const FVector& Size);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void RefreshPCGGeneration();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearAllGeneration();

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void AddBiomeConfiguration(const FBiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void RemoveBiomeConfiguration(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FBiomeConfiguration GetBiomeConfiguration(int32 Index) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    int32 GetBiomeConfigurationCount() const;

    // Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    EBiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World Generation")
    bool IsLocationInWater(const FVector& WorldLocation) const;

    // PCG Volume Management
    UFUNCTION(BlueprintCallable, Category = "PCG Management")
    APCGVolume* CreatePCGVolumeForBiome(const FBiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "PCG Management")
    void UpdatePCGVolume(APCGVolume* PCGVolume, const FBiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "PCG Management")
    TArray<APCGVolume*> GetAllPCGVolumes() const;

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> RootSceneComponent;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    TArray<FBiomeConfiguration> BiomeConfigurations;

    // PCG Graphs for different biomes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG Graphs")
    TMap<EBiomeType, TSoftObjectPtr<UPCGGraph>> BiomePCGGraphs;

    // World Generation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldSizeX = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldSizeY = 10000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float SeaLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    bool bUseWorldPartition = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    bool bAutoGenerateOnBeginPlay = true;

    // Runtime State
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<TObjectPtr<APCGVolume>> SpawnedPCGVolumes;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    bool bIsGenerationInProgress = false;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float GenerationProgress = 0.0f;

private:
    // Internal Functions
    void LoadBiomePCGGraphs();
    void SetupDefaultBiomeConfigurations();
    void CleanupPCGVolumes();
    
    // Biome Logic
    EBiomeType DetermineBiomeType(const FVector& Location, float Temperature, float Humidity, float Elevation) const;
    FVector2D WorldToNoiseCoordinates(const FVector& WorldLocation) const;
    
    // PCG Integration
    void OnPCGGenerationComplete(UPCGComponent* PCGComponent);
    
    // Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBiomeGenerationComplete, EBiomeType, BiomeType);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldGenerationComplete);
    
public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBiomeGenerationComplete OnBiomeGenerationComplete;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWorldGenerationComplete OnWorldGenerationComplete;
};