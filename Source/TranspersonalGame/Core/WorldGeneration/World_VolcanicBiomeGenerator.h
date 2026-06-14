#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_VolcanicBiomeGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Volcanic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Temperature = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float LavaFlowSpeed = 10.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LavaFlow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    TArray<FVector> FlowPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float FlowWidth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float FlowSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Temperature = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    bool bIsFlowing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float DamagePerSecond = 100.0f;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_VolcanicBiomeGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_VolcanicBiomeGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Volcanic terrain generation
    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void GenerateVolcanicTerrain(const FVector& CenterLocation, float Radius, int32 NumVolcanoes = 3);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void CreateVolcanicRocks(const FVector& Location, float Density = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void GenerateLavaFlows(const FVector& VolcanoLocation, int32 NumFlows = 5);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void CreateAshClouds(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void SpawnVolcanicVegetation(const FVector& Location, float SafeDistance = 200.0f);

    // Volcanic features management
    UFUNCTION(BlueprintCallable, Category = "Volcanic Features")
    void AddVolcanicFeature(const FWorld_VolcanicFeature& Feature);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Features")
    void RemoveVolcanicFeature(int32 FeatureIndex);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Features")
    TArray<FWorld_VolcanicFeature> GetVolcanicFeatures() const { return VolcanicFeatures; }

    // Lava flow simulation
    UFUNCTION(BlueprintCallable, Category = "Lava Simulation")
    void UpdateLavaFlows(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Lava Simulation")
    void CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Lava Simulation")
    bool IsLocationInLava(const FVector& Location) const;

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    bool IsLocationSafeForVegetation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void ApplyVolcanicWeathering(AActor* Actor, float WeatheringRate = 1.0f);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeVolcanicFeatures(const FVector& PlayerLocation, float CullingDistance = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetVolcanicLOD(int32 LODLevel);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    TArray<FWorld_VolcanicFeature> VolcanicFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    TArray<FWorld_LavaFlow> LavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float BaseTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float VolcanicInfluenceRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    int32 MaxVolcanicFeatures = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    int32 MaxLavaFlows = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    bool bEnableVolcanicSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Settings")
    float VolcanicUpdateInterval = 1.0f;

    // Mesh references for volcanic features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Meshes")
    TSoftObjectPtr<UStaticMesh> VolcanicRockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Meshes")
    TSoftObjectPtr<UStaticMesh> LavaRockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Meshes")
    TSoftObjectPtr<UMaterialInterface> LavaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Meshes")
    TSoftObjectPtr<UMaterialInterface> VolcanicRockMaterial;

private:
    float LastVolcanicUpdate = 0.0f;
    int32 CurrentLODLevel = 0;

    // Helper functions
    FVector CalculateLavaFlowDirection(const FVector& StartLocation, const FVector& EndLocation) const;
    float CalculateVolcanicInfluence(const FVector& Location, const FWorld_VolcanicFeature& Feature) const;
    void CleanupDistantVolcanicFeatures(const FVector& PlayerLocation, float MaxDistance);
};