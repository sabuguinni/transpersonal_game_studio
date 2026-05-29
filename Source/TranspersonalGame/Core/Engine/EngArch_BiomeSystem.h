#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "EngArch_BiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FEng_BiomeData()
    {
        BiomeType = EEng_BiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType FromBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType ToBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor;

    FEng_BiomeTransition()
    {
        FromBiome = EEng_BiomeType::Savanna;
        ToBiome = EEng_BiomeType::Forest;
        TransitionDistance = 1000.0f;
        BlendFactor = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetVegetationForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetDinosaursForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& Location, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetNearestBiomeCenter(const FVector& Location, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiome(const FEng_BiomeData& BiomeData);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    float DefaultBiomeRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    bool bBiomesInitialized;

private:
    void SetupDefaultBiomes();
    void SetupBiomeTransitions();
    float CalculateDistanceToBiome(const FVector& Location, const FEng_BiomeData& BiomeData) const;
    EEng_BiomeType DetermineBiomeFromEnvironmentalFactors(const FVector& Location) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_BiomeActor : public AActor
{
    GENERATED_BODY()

public:
    AEng_BiomeActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FEng_BiomeData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    class USphereComponent* BiomeBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    class UStaticMeshComponent* VisualizationMesh;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData() const { return BiomeData; }

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeData(const FEng_BiomeData& NewBiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& Location) const;
};