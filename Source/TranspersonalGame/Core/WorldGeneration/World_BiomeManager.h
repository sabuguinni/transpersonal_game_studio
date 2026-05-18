#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 ActorDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> TerrainAssets;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 50000.0f;
        ActorDensity = 100;
    }
};

USTRUCT(BlueprintType)
struct FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FString Name;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_BiomeType ParentBiome;

    FWorld_WaterBody()
    {
        Name = TEXT("Water");
        Location = FVector::ZeroVector;
        Scale = FVector(100.0f, 100.0f, 1.0f);
        ParentBiome = EWorld_BiomeType::Savana;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    TArray<FWorld_WaterBody> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    bool bAutoGenerateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxActorsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinSpawnDistance;

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void PopulateBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void CreateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void CreateTerrainFeatures();

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    int32 GetActorCountInBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Generation")
    void ClearBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintBiomeStats();

private:
    void SetupDefaultBiomes();
    void SetupDefaultWaterBodies();
    FVector GetRandomLocationInBiome(const FWorld_BiomeData& BiomeData) const;
    bool IsLocationValid(const FVector& Location, const FWorld_BiomeData& BiomeData) const;

    UPROPERTY()
    TArray<AStaticMeshActor*> SpawnedActors;
};