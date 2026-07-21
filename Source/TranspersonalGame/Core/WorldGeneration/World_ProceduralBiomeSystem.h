#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
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
    float BaseHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> RockFormations;

    FWorld_BiomeDefinition()
    {
        BiomeName = TEXT("Unknown");
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        BaseHeight = 100.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_ProceduralSpawnParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MinCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MinScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float MaxScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    float SpawnRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    bool bRandomRotation;

    FWorld_ProceduralSpawnParams()
    {
        MinCount = 5;
        MaxCount = 15;
        MinScale = 0.8f;
        MaxScale = 2.0f;
        SpawnRadius = 0.8f;
        bRandomRotation = true;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FWorld_ProceduralSpawnParams RockSpawnParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FWorld_ProceduralSpawnParams VegetationSpawnParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FWorld_ProceduralSpawnParams DinosaurSpawnParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxSpawnDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedActors;

public:
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FWorld_BiomeDefinition GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    TArray<FString> GetAllBiomeNames() const;

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    int32 GetActorCountInBiome(const FString& BiomeName) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance(const FString& BiomeName, float PlayerDistance);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void EditorGenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void EditorClearAllBiomes();

protected:
    void SpawnRockFormations(const FWorld_BiomeDefinition& Biome);
    void SpawnVegetation(const FWorld_BiomeDefinition& Biome);
    void SpawnDinosaurs(const FWorld_BiomeDefinition& Biome);
    
    FVector GetRandomLocationInBiome(const FWorld_BiomeDefinition& Biome, float RadiusMultiplier = 0.8f) const;
    FRotator GetRandomRotation() const;
    float GetRandomScale(float MinScale, float MaxScale) const;
    
    void CleanupActorsInBiome(const FString& BiomeName);
    bool IsActorInBiome(AActor* Actor, const FWorld_BiomeDefinition& Biome) const;
};