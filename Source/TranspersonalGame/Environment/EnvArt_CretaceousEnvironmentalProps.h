#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousEnvironmentalProps.generated.h"

/**
 * Environmental prop spawner and manager for Cretaceous period atmosphere
 * Handles fallen logs, boulders, weathered rocks, and atmospheric storytelling elements
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousEnvironmentalProps : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousEnvironmentalProps();

protected:
    virtual void BeginPlay() override;

    // === PROP MESH COMPONENTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cretaceous Props")
    UStaticMeshComponent* FallenLogMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cretaceous Props")
    UStaticMeshComponent* WeatheredBoulderMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cretaceous Props")
    UStaticMeshComponent* AncientStumpMesh;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cretaceous Props")
    UStaticMeshComponent* FossilizedLogMesh;

    // === PROP CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Props")
    TArray<FVector> PropSpawnLocations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Props")
    float PropDensity = 0.8f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Props")
    float ScaleVariation = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Props")
    bool bEnableWeatheringEffects = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Props")
    float MossGrowthIntensity = 0.6f;

    // === ATMOSPHERIC STORYTELLING ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    bool bEnableEnvironmentalStorytelling = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    TArray<FString> StorytellingPropTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storytelling")
    float StorytellingRadius = 500.0f;

public:
    // === PROP SPAWNING METHODS ===
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnCretaceousProps();
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnFallenLog(const FVector& Location, const FRotator& Rotation);
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnWeatheredBoulder(const FVector& Location, float Scale = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnAncientStump(const FVector& Location, const FRotator& Rotation);
    
    UFUNCTION(BlueprintCallable, Category = "Environmental Props")
    void SpawnFossilizedLog(const FVector& Location, const FRotator& Rotation);
    
    // === ATMOSPHERIC ENHANCEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyWeatheringEffects();
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateMossGrowth();
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateEnvironmentalStorytellingCluster(const FVector& CenterLocation);
    
    // === BIOME INTEGRATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void AdaptPropsToCurrentBiome(EBiomeType BiomeType);
    
    UFUNCTION(BlueprintCallable, Category = "Biome Integration")
    void UpdatePropsForWeatherConditions(EWeatherType WeatherType);

private:
    // === INTERNAL PROP MANAGEMENT ===
    
    void InitializePropMeshes();
    void SetupPropCollision();
    void ApplyRandomVariation(UStaticMeshComponent* MeshComponent);
    void CreateStorytellingNarrative(const FVector& Location, const FString& PropType);
    
    // === PERFORMANCE OPTIMIZATION ===
    
    UPROPERTY()
    TArray<AActor*> SpawnedProps;
    
    UPROPERTY()
    float LastUpdateTime = 0.0f;
    
    UPROPERTY()
    bool bPropsInitialized = false;
};