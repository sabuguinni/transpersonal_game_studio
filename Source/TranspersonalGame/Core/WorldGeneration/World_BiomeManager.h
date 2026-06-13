#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Tundra      UMETA(DisplayName = "Tundra"),
    Coastal     UMETA(DisplayName = "Coastal"),
    Plains      UMETA(DisplayName = "Plains")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f; // 0-1 range

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ResourceDensity = 1.0f;

    FWorld_BiomeData()
    {
        VegetationTypes.Add("GenericTree");
        DinosaurSpecies.Add("GenericHerbivore");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bAutoGenerateBiomes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    int32 MaxBiomeCount = 8;

public:
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetVegetationForBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetDinosaursForBiome(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void AddBiomeZone(const FWorld_BiomeData& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetBiomeInfluenceAtLocation(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

private:
    void InitializeDefaultBiomes();
    FWorld_BiomeData CreateBiomeData(EWorld_BiomeType Type, const FVector& Center, float Radius);
    float CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& Biome) const;
};