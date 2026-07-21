#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "BiomeSystemArchitecture.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Vegetation = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurCount = 50;

    FEng_BiomeConfiguration()
    {
        AllowedDinosaurSpecies.Add("Trex");
        AllowedDinosaurSpecies.Add("Velociraptor");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FEng_BiomeConfiguration> BiomeConfigurations;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    TArray<FVector> GetSpawnLocationsInBiome(EBiomeType BiomeType, int32 Count) const;

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void PopulateBiomeWithActors(EBiomeType BiomeType, const TArray<FString>& AssetPaths);

protected:
    virtual void BeginPlay() override;

private:
    void SetupDefaultBiomes();
    FVector GetRandomLocationInBiome(const FEng_BiomeConfiguration& BiomeConfig) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeActor : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* BiomeMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UEng_BiomeManager* BiomeManagerComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType AssignedBiomeType = EBiomeType::Savana;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeType(EBiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SpawnDinosaursInBiome();

protected:
    virtual void BeginPlay() override;
};