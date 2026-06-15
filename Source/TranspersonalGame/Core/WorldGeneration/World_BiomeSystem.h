#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Default Biome");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UMaterialInterface>> GroundMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    FWorld_BiomeData()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Forest");
        Temperature = 20.0f;
        Humidity = 0.6f;
        Elevation = 100.0f;
        VegetationDensity = 1.0f;
        FogColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendStrength = 0.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome data management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EBiomeType, FWorld_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeCheckRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    float BiomeUpdateInterval = 2.0f;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    EBiomeType CurrentBiome = EBiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    FVector CurrentBiomeCenter = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    float BiomeInfluenceStrength = 1.0f;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateCurrentBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float CalculateBiomeInfluence(const FVector& Location, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeDatabase();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<AActor*> GetBiomeActorsInRadius(const FVector& Center, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ApplyBiomeEffectsToActor(AActor* TargetActor, EBiomeType BiomeType, float Intensity);

    // Biome generation helpers
    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void GenerateBiomeVegetation(const FVector& Center, float Radius, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void ApplyBiomeAtmosphere(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void CreateBiomeTransitionZone(const FVector& Location, EBiomeType FromBiome, EBiomeType ToBiome);

private:
    float LastBiomeUpdateTime = 0.0f;
    TMap<EBiomeType, TArray<AActor*>> CachedBiomeActors;

    void UpdateBiomeCache();
    float CalculateNoiseValue(const FVector& Location, float Scale, int32 Octaves);
    FVector GetBiomeNoiseOffset(EBiomeType BiomeType);
};