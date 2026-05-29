#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurAssetPaths;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("DefaultBiome");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        DinosaurDensity = 0.1f;
        VegetationDensity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_SpawnedActorInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    AActor* SpawnedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString ActorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float DistanceFromPlayer;

    FWorld_SpawnedActorInfo()
    {
        SpawnedActor = nullptr;
        BiomeName = TEXT("Unknown");
        ActorType = TEXT("Unknown");
        SpawnLocation = FVector::ZeroVector;
        DistanceFromPlayer = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core biome management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float CullingDistance;

    // Spawned actor tracking
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<FWorld_SpawnedActorInfo> SpawnedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    int32 TotalSpawnedCount;

    // Biome population functions
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void PopulateBiome(const FWorld_BiomeDefinition& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    AActor* SpawnDinosaurInBiome(const FString& AssetPath, const FVector& Location, const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    AActor* SpawnVegetationInBiome(const FString& VegetationType, const FVector& Location, const FString& BiomeName);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorsByDistance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FVector GetPlayerLocation() const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    FWorld_BiomeDefinition* GetBiomeAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<AActor*> GetActorsInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void PrintBiomeStatistics() const;

    // Editor functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void InitializeDefaultBiomes();

    UFUNCTION(CallInEditor, Category = "Editor")
    void ClearAllSpawnedActors();

private:
    // Internal state
    float LastUpdateTime;
    int32 CurrentSpawnIndex;
    bool bIsPopulating;

    // Helper functions
    FVector GenerateRandomLocationInBiome(const FWorld_BiomeDefinition& Biome) const;
    bool IsLocationValidForSpawn(const FVector& Location) const;
    void RegisterSpawnedActor(AActor* Actor, const FString& BiomeName, const FString& ActorType, const FVector& Location);
    void UpdateActorDistances();
};