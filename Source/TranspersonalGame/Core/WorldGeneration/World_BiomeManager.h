#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "../SharedTypes.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Desert      UMETA(DisplayName = "Desert"),
    Tundra      UMETA(DisplayName = "Tundra")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityModifier = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AudioZones;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeColor = FLinearColor::Green;
        VegetationDensity = 0.5f;
        TemperatureModifier = 0.0f;
        HumidityModifier = 0.0f;
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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* BiomeMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* BiomeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 MaxVegetationPerBiome = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float BiomeTransitionBlendDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePerformanceOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance = 5000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void SetBiomeAudio(EWorld_BiomeType BiomeType, const FString& AudioZoneName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetationForBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateBiomeTransitionZone(const FVector& Location1, const FVector& Location2);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeBiomePerformance(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherForBiome(EWorld_BiomeType BiomeType, float Temperature, float Humidity);

    UFUNCTION(BlueprintPure, Category = "Biome Query")
    float GetDistanceToBiome(const FVector& Location, EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintPure, Category = "Biome Query")
    bool IsLocationInBiome(const FVector& Location, EWorld_BiomeType BiomeType) const;

private:
    void CreateDefaultBiomes();
    void SpawnBiomeMarkers();
    void SetupAudioZones();
    FVector GetRandomLocationInBiome(EWorld_BiomeType BiomeType) const;
    void ApplyBiomeSpecificSettings(EWorld_BiomeType BiomeType);
};