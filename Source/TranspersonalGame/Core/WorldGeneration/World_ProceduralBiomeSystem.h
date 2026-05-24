#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "../../SharedTypes.h"
#include "World_ProceduralBiomeSystem.generated.h"

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
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> TerrainAssetPaths;

    FWorld_BiomeConfig()
    {
        BiomeName = TEXT("Default");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000000.0f; // 10km
        MaxDinosaurs = 10;
        MaxTerrainFeatures = 50;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_SpawnedActor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    TSoftObjectPtr<AActor> ActorRef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString ActorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnTime;

    FWorld_SpawnedActor()
    {
        BiomeName = TEXT("Unknown");
        ActorType = TEXT("Unknown");
        SpawnLocation = FVector::ZeroVector;
        SpawnTime = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_ProceduralBiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_ProceduralBiomeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TArray<FWorld_SpawnedActor> SpawnedActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bAutoPopulateBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float PopulationUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSpawnsPerFrame;

private:
    FTimerHandle PopulationTimerHandle;
    int32 CurrentSpawnIndex;
    float LastUpdateTime;

public:
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void PopulateBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    int32 GetBiomePopulation(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FWorld_BiomeConfig GetBiomeConfig(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<AActor*> GetActorsInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    AActor* SpawnActorInBiome(const FString& BiomeName, const FString& AssetPath, const FVector& Offset = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Spawning")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateActorCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorPopulateBiomes();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorClearBiomes();

    UFUNCTION(CallInEditor, Category = "Editor")
    void EditorValidateBiomes();

private:
    void UpdatePopulation();
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfig& BiomeConfig) const;
    bool IsLocationValid(const FVector& Location, const FString& BiomeName) const;
    void SetupDefaultBiomes();
    void RegisterSpawnedActor(AActor* Actor, const FString& BiomeName, const FString& ActorType);
    void CleanupInvalidActors();
};