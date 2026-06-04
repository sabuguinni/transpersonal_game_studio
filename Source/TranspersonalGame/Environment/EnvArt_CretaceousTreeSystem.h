#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_CretaceousTreeSystem.generated.h"

class UStaticMeshComponent;
class UMaterialInterface;
class UNiagaraComponent;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_CretaceousTreeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float TreeHeight = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float TrunkDiameter = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float CanopyRadius = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float Age = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    bool bIsFallenLog = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float MossGrowth = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Data")
    float WeatheringLevel = 0.5f;
};

UENUM(BlueprintType)
enum class EEnvArt_CretaceousTreeSpecies : uint8
{
    Araucaria,      // Monkey puzzle tree
    Ginkgo,         // Ginkgo biloba
    Cycad,          // Palm-like cycads
    Fern,           // Tree ferns
    Conifer,        // Various conifers
    Redwood,        // Early redwoods
    FallenLog       // Decomposing fallen trees
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_CretaceousTreeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_CretaceousTreeSystem();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tree Management
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void SpawnTreeCluster(const FVector& Location, EEnvArt_CretaceousTreeSpecies Species, int32 Count = 5);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void SpawnFallenLog(const FVector& Location, const FRotator& Rotation);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void UpdateTreeSeason(float SeasonProgress);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void ApplyWeatherEffects(float RainIntensity, float WindStrength);

    // Environmental Interaction
    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void AddMossGrowth(AActor* TreeActor, float GrowthAmount);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void CreateBirdNest(AActor* TreeActor, const FVector& NestLocation);

    UFUNCTION(BlueprintCallable, Category = "Cretaceous Trees")
    void SpawnInsectSwarm(const FVector& Location);

protected:
    // Tree Species Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tree Species")
    TMap<EEnvArt_CretaceousTreeSpecies, FEnvArt_CretaceousTreeData> TreeSpeciesData;

    // Mesh Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TMap<EEnvArt_CretaceousTreeSpecies, TSoftObjectPtr<UStaticMesh>> TreeMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    TSoftObjectPtr<UStaticMesh> FallenLogMesh;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> BarkMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> MossyBarkMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TSoftObjectPtr<UMaterialInterface> FoliageMaterial;

    // Particle Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<class UNiagaraSystem> PollenParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<class UNiagaraSystem> FallingLeavesParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    TSoftObjectPtr<class UNiagaraSystem> InsectSwarmParticles;

    // Spawning Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MinTreeSpacing = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float MaxTreeSpacing = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float TerrainAdaptation = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxTreesPerCluster = 12;

    // Environmental State
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentSeasonProgress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentRainIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    float CurrentWindStrength = 0.0f;

private:
    // Internal tracking
    TArray<TWeakObjectPtr<AActor>> SpawnedTrees;
    TArray<TWeakObjectPtr<AActor>> SpawnedLogs;
    TArray<TWeakObjectPtr<UNiagaraComponent>> ActiveParticleEffects;

    // Helper functions
    FVector GetRandomSpawnLocation(const FVector& CenterLocation, float Radius);
    FRotator GetTerrainAlignedRotation(const FVector& Location);
    void ApplySpeciesCharacteristics(AActor* TreeActor, EEnvArt_CretaceousTreeSpecies Species);
    void UpdateTreeMaterial(AActor* TreeActor, float MossGrowth, float Weathering);
    bool IsValidTreeLocation(const FVector& Location, float MinDistance);
};