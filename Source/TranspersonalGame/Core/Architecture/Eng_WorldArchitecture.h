#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_WorldArchitecture.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_WorldZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    int32 MaxDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    TArray<EDinosaurSpecies> AllowedSpecies;

    FEng_WorldZone()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        BiomeType = EBiomeType::Grassland;
        MaxDinosaurs = 20;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_TerrainSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float HeightScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    int32 Octaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Persistence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Lacunarity;

    FEng_TerrainSettings()
    {
        HeightScale = 1000.0f;
        NoiseScale = 0.001f;
        Octaves = 4;
        Persistence = 0.5f;
        Lacunarity = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_WorldArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_WorldArchitecture();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void InitializeWorldZones();

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void RegisterWorldZone(const FEng_WorldZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    FEng_WorldZone GetZoneAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    TArray<FEng_WorldZone> GetAllZones() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void SetTerrainSettings(const FEng_TerrainSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    FEng_TerrainSettings GetTerrainSettings() const;

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    bool IsLocationInValidZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "World Architecture")
    void ValidateWorldIntegrity();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FEng_WorldZone> WorldZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FEng_TerrainSettings TerrainSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float WorldBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    int32 MaxTotalActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bWorldInitialized;

private:
    void CreateDefaultZones();
    void ValidateZoneOverlaps();
    float CalculateZoneDistance(const FEng_WorldZone& ZoneA, const FEng_WorldZone& ZoneB);
};