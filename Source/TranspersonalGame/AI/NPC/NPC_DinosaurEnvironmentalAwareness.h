#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "SharedTypes.h"
#include "NPC_DinosaurEnvironmentalAwareness.generated.h"

USTRUCT(BlueprintType)
struct FNPC_EnvironmentalFactor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    ENPCEnvironmentalType FactorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float Duration;

    FNPC_EnvironmentalFactor()
    {
        FactorType = ENPCEnvironmentalType::Neutral;
        Intensity = 0.5f;
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        Duration = 10.0f;
    }
};

/**
 * Environmental awareness system for dinosaur NPCs
 * Tracks weather, terrain, resources, and other environmental factors that influence behavior
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_DinosaurEnvironmentalAwareness : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_DinosaurEnvironmentalAwareness();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Environmental scanning
    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void ScanEnvironment();

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateWeatherAwareness();

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateTerrainAwareness();

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void UpdateResourceAwareness();

    // Environmental factor management
    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void AddEnvironmentalFactor(const FNPC_EnvironmentalFactor& Factor);

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    void RemoveEnvironmentalFactor(ENPCEnvironmentalType FactorType);

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    float GetEnvironmentalInfluence(ENPCEnvironmentalType FactorType) const;

    // Pathfinding assistance
    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    bool IsLocationSafe(const FVector& Location, float SafetyRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    FVector FindSafeLocation(const FVector& StartLocation, float SearchRadius = 2000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    TArray<FVector> GetAvoidancePoints() const;

    // Resource detection
    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    TArray<FVector> FindNearbyResources(ENPCResourceType ResourceType, float SearchRadius = 1500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    FVector FindNearestWaterSource(float SearchRadius = 3000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    FVector FindNearestShelter(float SearchRadius = 2000.0f) const;

    // Environmental state queries
    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    float GetComfortLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    ENPCWeatherType GetCurrentWeather() const { return CurrentWeather; }

    UFUNCTION(BlueprintCallable, Category = "Environmental Awareness")
    ENPCTerrainType GetCurrentTerrain() const { return CurrentTerrain; }

protected:
    // Environmental factors
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental State")
    TArray<FNPC_EnvironmentalFactor> ActiveFactors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental State")
    ENPCWeatherType CurrentWeather;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental State")
    ENPCTerrainType CurrentTerrain;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental State")
    float EnvironmentalStress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental State")
    float ComfortLevel;

    // Scanning parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanning Parameters")
    float ScanRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanning Parameters")
    float ScanInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanning Parameters")
    float WeatherSensitivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scanning Parameters")
    float TerrainSensitivity;

    // Resource preferences
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Preferences")
    TMap<ENPCResourceType, float> ResourceImportance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Preferences")
    float WaterNeedThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Preferences")
    float ShelterNeedThreshold;

    // Avoidance zones
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avoidance")
    TArray<FVector> DangerZones;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Avoidance")
    TArray<FVector> AvoidancePoints;

    // Timing
    float LastScanTime;
    float LastWeatherUpdate;
    float LastTerrainUpdate;

private:
    // Internal scanning functions
    void ScanForDangers();
    void ScanForResources();
    void ScanForShelter();
    void UpdateEnvironmentalStress();
    void ProcessEnvironmentalFactors(float DeltaTime);
    
    // Helper functions
    bool IsLocationInDangerZone(const FVector& Location, float SafetyMargin = 100.0f) const;
    float CalculateLocationSafety(const FVector& Location) const;
    ENPCWeatherType DetermineCurrentWeather() const;
    ENPCTerrainType DetermineCurrentTerrain() const;
    void UpdateComfortLevel();
};