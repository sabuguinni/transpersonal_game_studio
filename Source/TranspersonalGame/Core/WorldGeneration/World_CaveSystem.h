#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/PointLight.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "World_CaveSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_CaveEntrance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    float EntranceWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    float EntranceHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    EWorld_BiomeType ConnectedBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Entrance")
    bool bHasUndergroundWater;

    FWorld_CaveEntrance()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        EntranceWidth = 300.0f;
        EntranceHeight = 250.0f;
        ConnectedBiome = EWorld_BiomeType::Forest;
        bHasUndergroundWater = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_UndergroundAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EWorld_AudioType AudioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float EchoIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float ReverbAmount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    USoundCue* AmbientSound;

    FWorld_UndergroundAudioZone()
    {
        Position = FVector::ZeroVector;
        Radius = 500.0f;
        AudioType = EWorld_AudioType::Cave;
        EchoIntensity = 0.7f;
        ReverbAmount = 0.8f;
        AmbientSound = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_UndergroundWaterFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    float FlowRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    bool bHasDrippingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    float WaterTemperature;

    FWorld_UndergroundWaterFeature()
    {
        Location = FVector::ZeroVector;
        Scale = FVector(5.0f, 5.0f, 1.0f);
        WaterType = EWorld_WaterType::UndergroundPool;
        FlowRate = 0.0f;
        bHasDrippingSound = true;
        WaterTemperature = 15.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CaveSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CaveSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Cave entrance management
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateCaveEntrances(int32 NumberOfCaves);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateCaveEntrance(const FWorld_CaveEntrance& CaveData);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    TArray<FWorld_CaveEntrance> GetCaveEntrances() const;

    // Underground audio zone management
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void SetupUndergroundAudio();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateAudioZone(const FWorld_UndergroundAudioZone& AudioZone);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void UpdateAudioZoneEffects(float DeltaTime);

    // Underground water feature management
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void GenerateUndergroundWater();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void CreateWaterFeature(const FWorld_UndergroundWaterFeature& WaterData);

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void UpdateWaterFlow(float DeltaTime);

    // Cave system validation
    UFUNCTION(BlueprintCallable, Category = "Cave System")
    bool ValidateCaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Cave System")
    void OptimizeCavePerformance();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<FWorld_CaveEntrance> CaveEntrances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<FWorld_UndergroundAudioZone> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    TArray<FWorld_UndergroundWaterFeature> WaterFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    int32 MaxCaveEntrances;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float MinDistanceBetweenCaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float CaveDepthRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bEnableUndergroundAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    bool bEnableUndergroundWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float AudioUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Configuration")
    float WaterUpdateInterval;

private:
    // Spawned actors
    UPROPERTY()
    TArray<AActor*> SpawnedCaveActors;

    UPROPERTY()
    TArray<AActor*> SpawnedAudioActors;

    UPROPERTY()
    TArray<AActor*> SpawnedWaterActors;

    // Timing
    float AudioUpdateTimer;
    float WaterUpdateTimer;

    // Helper functions
    FVector FindValidCaveLocation(EWorld_BiomeType BiomeType);
    bool IsCaveLocationValid(const FVector& Location);
    void SpawnCaveEntranceActor(const FWorld_CaveEntrance& CaveData);
    void SpawnAudioZoneActor(const FWorld_UndergroundAudioZone& AudioZone);
    void SpawnWaterFeatureActor(const FWorld_UndergroundWaterFeature& WaterData);
    void CleanupSpawnedActors();
};