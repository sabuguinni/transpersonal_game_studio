#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_AdvancedBiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float BiomeRadius = 25000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 VegetationDensity = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 RockDensity = 200;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    int32 DinosaurCount = 15;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<FString> VegetationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<FString> RockAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    TArray<FString> DinosaurAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float TemperatureRange = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Config")
    float HumidityLevel = 0.5f;

    FWorld_BiomeConfiguration()
    {
        VegetationAssets = {
            TEXT("/Game/LandscapePackOne/Meshes/SM_Tree_01"),
            TEXT("/Game/LandscapePackOne/Meshes/SM_Tree_02"),
            TEXT("/Game/LandscapePackOne/Meshes/SM_Bush_01")
        };
        
        RockAssets = {
            TEXT("/Game/LandscapePackOne/Meshes/SM_Rock_01"),
            TEXT("/Game/LandscapePackOne/Meshes/SM_Rock_02")
        };
        
        DinosaurAssets = {
            TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
            TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin")
        };
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_SpawnResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Spawn Result")
    int32 VegetationSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Spawn Result")
    int32 RocksSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Spawn Result")
    int32 DinosaursSpawned = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Spawn Result")
    float ExecutionTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Spawn Result")
    bool bSuccess = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_AdvancedBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_AdvancedBiomeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TArray<FWorld_SpawnResult> SpawnResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float SpawnDelay = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bIsGenerating = false;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 CurrentBiomeIndex = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateBiome(const FWorld_BiomeConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_SpawnResult PopulateBiomeArea(const FWorld_BiomeConfiguration& Config);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnVegetationInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnRocksInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnDinosaursInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationValid(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void SetupDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void ClearAllBiomeActors();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeStatus();

private:
    void CreateDefaultBiomeConfigurations();
    AActor* SpawnActorFromAssetPath(const FString& AssetPath, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);
    FTimerHandle GenerationTimerHandle;
    void ContinueGeneration();
};