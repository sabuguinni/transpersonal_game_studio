#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    RiverDelta      UMETA(DisplayName = "River Delta"),
    JungleLowland   UMETA(DisplayName = "Jungle Lowland"),
    RockyHighland   UMETA(DisplayName = "Rocky Highland"),
    VolcanicBadland UMETA(DisplayName = "Volcanic Badland"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    Unknown         UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float DangerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FString BiomeName = TEXT("Unknown Biome");
};

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Query which biome a world location falls in
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get the full biome zone data for a location
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    bool GetBiomeZoneAtLocation(const FVector& WorldLocation, FWorld_BiomeZone& OutZone) const;

    // Get ambient temperature at location (affects player survival stats)
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Get danger level at location (affects dinosaur aggression radius)
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // Register a new biome zone at runtime (called by PCG)
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void RegisterBiomeZone(const FWorld_BiomeZone& Zone);

    // Get all registered biome zones
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const;

    // Called by PCGWorldGenerator to initialize default biomes
    UFUNCTION(CallInEditor, Category = "World|Biome")
    void InitializeDefaultBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bAutoInitializeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float BiomeTransitionBlendRadius = 500.0f;

private:
    // Find nearest biome zone to a location
    int32 FindNearestBiomeIndex(const FVector& WorldLocation) const;
};
