#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_BiomeZoneController.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogBiomeZoneController, Log, All);

/**
 * Biome zone configuration for massive world generation
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float TemperatureRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float HumidityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zone")
    float ElevationModifier;

    FWorld_BiomeZoneData()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        BiomeType = EBiomeType::Temperate;
        TemperatureRange = 25.0f;
        HumidityLevel = 0.5f;
        ElevationModifier = 0.0f;
    }
};

/**
 * Controls and manages biome zones across the massive 10km x 10km landscape
 * Implements the 5 distinct biome zones required for asset purchase criteria
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeZoneController : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeZoneController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core biome zone management
    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    void InitializeBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    void CreateBiomeZoneActors();

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    EBiomeType GetBiomeTypeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    FWorld_BiomeZoneData GetBiomeZoneData(const FVector& WorldLocation) const;

    // Biome zone validation for asset criteria
    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    bool ValidateBiomeZoneConfiguration() const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    TArray<FWorld_BiomeZoneData> GetAllBiomeZones() const;

    // Distance and transition calculations
    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    float GetDistanceToNearestBiomeCenter(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    float GetBiomeTransitionFactor(const FVector& WorldLocation) const;

    // Environmental effects per biome
    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Zone Controller")
    float GetElevationModifierAtLocation(const FVector& WorldLocation) const;

protected:
    // The 5 required biome zones for asset purchase criteria
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Zones", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeZoneData> BiomeZones;

    // Zone generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float BiomeTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    bool bAutoCreateZoneActors;

    // Runtime state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    bool bZonesInitialized;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime", meta = (AllowPrivateAccess = "true"))
    int32 ActiveZoneCount;

private:
    // Internal helper functions
    void SetupDefaultBiomeZones();
    FWorld_BiomeZoneData CreateBiomeZone(const FString& Name, const FVector& Location, float Radius, EBiomeType Type, float Temperature, float Humidity, float Elevation);
    float CalculateZoneInfluence(const FVector& WorldLocation, const FWorld_BiomeZoneData& ZoneData) const;
    FWorld_BiomeZoneData GetDominantBiomeZone(const FVector& WorldLocation) const;
    void SpawnBiomeZoneActor(const FWorld_BiomeZoneData& ZoneData);
    bool ValidateZoneSpacing() const;
    bool ValidateZoneCoverage() const;
};