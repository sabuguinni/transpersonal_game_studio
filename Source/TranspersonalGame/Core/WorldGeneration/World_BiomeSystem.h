#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"), 
    Rocky       UMETA(DisplayName = "Rocky"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Desert      UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<class UStaticMesh*> VegetationMeshes;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Forest;
        VegetationDensity = 1.0f;
        Temperature = 25.0f;
        Humidity = 0.6f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* BiomeVolume;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components") 
    class UStaticMeshComponent* BiomeMarker;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float RainIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float WindStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    float FogDensity = 0.1f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeType() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void SetBiomeType(EWorld_BiomeType NewBiomeType);

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void UpdateWeatherConditions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetation();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsPointInBiome(const FVector& WorldPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetEnvironmentalFactor(const FString& FactorName) const;
};