#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    RockyHills      UMETA(DisplayName = "Rocky Hills"),
    SwampLands      UMETA(DisplayName = "Swamp Lands"),
    VolcanicRidge   UMETA(DisplayName = "Volcanic Ridge"),
    RiverSystem     UMETA(DisplayName = "River System"),
    LakeSystem      UMETA(DisplayName = "Lake System")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AtmosphereColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;

    FWorld_BiomeConfig()
    {
        VegetationTypes.Add("GenericTree");
        DinosaurSpecies.Add("GenericHerbivore");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector ZoneExtents = FVector(1000.0f, 1000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float LoadDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float UnloadDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<EWorld_BiomeType> AssociatedBiomes;
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame))
class TRANSPERSONALGAME_API UWorld_BiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiome(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FWorld_BiomeConfig> GetAllBiomes() const { return BiomeConfigs; }

    // Vegetation Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void PlaceVegetationInBiome(EWorld_BiomeType BiomeType, int32 VegetationCount);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ClearVegetationInBiome(EWorld_BiomeType BiomeType);

    // Water System
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateWaterBody(const FVector& StartLocation, const FVector& EndLocation, float Width);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateLake(const FVector& Location, float Radius);

    // Streaming Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetupStreamingZones();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateStreamingZone(const FWorld_StreamingZone& StreamingZone);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInStreamingZone(const FVector& Location, const FWorld_StreamingZone& Zone) const;

    // Terrain Modification
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateTerrainFeature(const FVector& Location, float Radius, float Height, const FString& FeatureName);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ApplyBiomeTerrainModifications();

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeAtmosphere(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeLighting(EWorld_BiomeType BiomeType, const FLinearColor& LightColor, float Intensity);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void ValidateBiomeSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void DebugDrawBiomes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_StreamingZone> StreamingZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float BiomeTransitionDistance = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableStreamingOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bEnableAtmosphereEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float VegetationCullingDistance = 5000.0f;

    // Runtime Data
    UPROPERTY(Transient)
    TMap<EWorld_BiomeType, TArray<AActor*>> SpawnedVegetation;

    UPROPERTY(Transient)
    TArray<AActor*> WaterBodies;

    UPROPERTY(Transient)
    TArray<AActor*> TerrainFeatures;

    UPROPERTY(Transient)
    bool bIsInitialized = false;

private:
    // Helper Functions
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfig& BiomeConfig) const;
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeConfig& BiomeConfig) const;
    void SpawnVegetationActor(const FVector& Location, const FString& VegetationType, EWorld_BiomeType BiomeType);
    void CleanupBiomeActors();
};