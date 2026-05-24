#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Size = FVector(1000.0f, 1000.0f, 200.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EWorld_VegetationType> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<USoundCue> AmbientSound;

    FWorld_BiomeData()
    {
        VegetationTypes.Add(EWorld_VegetationType::Tree);
        VegetationTypes.Add(EWorld_VegetationType::Bush);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_VegetationType VegetationType = EWorld_VegetationType::Tree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 Count = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ScaleRange = FVector2D(1.0f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float HeightOffset = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> VegetationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UMaterial> VegetationMaterial;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TMap<EWorld_BiomeType, TArray<FWorld_VegetationConfig>> VegetationConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 RandomSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float BiomeTransitionDistance = 200.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> GeneratedBiomeActors;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> GeneratedVegetationActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateVegetationForBiome(EWorld_BiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void ClearGeneratedContent();

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Query")
    float GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateAmbientAudioForLocation(const FVector& ListenerLocation);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor Tools")
    void RegenerateWorld();

protected:
    UFUNCTION()
    void InitializeDefaultBiomes();

    UFUNCTION()
    void InitializeVegetationConfigs();

    UFUNCTION()
    AActor* SpawnBiomeMarker(const FWorld_BiomeData& BiomeData);

    UFUNCTION()
    AActor* SpawnVegetationActor(const FWorld_VegetationConfig& Config, const FVector& Location, const FRotator& Rotation);

    UFUNCTION()
    FVector GetRandomLocationInBiome(const FVector& BiomeCenter, float Radius) const;

    UFUNCTION()
    float CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const;

private:
    TMap<EWorld_BiomeType, UAudioComponent*> BiomeAudioComponents;
    
    void SetupBiomeAudio();
    void UpdateAudioComponentVolumes(const FVector& ListenerLocation);
};