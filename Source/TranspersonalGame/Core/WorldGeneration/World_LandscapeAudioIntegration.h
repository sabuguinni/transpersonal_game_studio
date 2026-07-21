#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_LandscapeAudioIntegration.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogLandscapeAudio, Log, All);

/**
 * World_LandscapeAudioIntegration
 * Integrates environmental audio with landscape terrain features
 * Creates dynamic audio zones based on biome transitions and terrain height
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_LandscapeAudioIntegration : public AActor
{
    GENERATED_BODY()

public:
    AWorld_LandscapeAudioIntegration();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Integration", meta = (AllowPrivateAccess = "true"))
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Integration", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Integration", meta = (AllowPrivateAccess = "true"))
    class UAudioComponent* SecondaryAudioComponent;

    // Landscape Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Audio")
    class ALandscape* TargetLandscape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Audio")
    float AudioUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Audio")
    float TerrainSampleRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Landscape Audio")
    int32 MaxAudioZones;

    // Biome Audio Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, class USoundBase*> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, float> BiomeAudioVolumes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EWorld_BiomeType, float> BiomeAudioPitches;

    // Height-Based Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Audio")
    class USoundBase* HighAltitudeWindSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Audio")
    class USoundBase* LowAltitudeWaterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Audio")
    float HighAltitudeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Height Audio")
    float LowAltitudeThreshold;

    // Dynamic Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EWorld_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTerrainHeight;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentAudioBlendWeight;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsNearWater;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsHighAltitude;

public:
    // Landscape Audio Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    void InitializeLandscapeAudio(ALandscape* InLandscape);

    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    void UpdateAudioBasedOnTerrain(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    float GetTerrainHeightAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    void SetBiomeAudio(EWorld_BiomeType BiomeType, float BlendWeight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Landscape Audio")
    void BlendBiomeAudio(EWorld_BiomeType FromBiome, EWorld_BiomeType ToBiome, float BlendAlpha);

    // Height-Based Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Height Audio")
    void UpdateHeightBasedAudio(float TerrainHeight);

    UFUNCTION(BlueprintCallable, Category = "Height Audio")
    void SetHighAltitudeAudio(bool bEnable, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Height Audio")
    void SetLowAltitudeAudio(bool bEnable, float Volume = 1.0f);

    // Water Detection Functions
    UFUNCTION(BlueprintCallable, Category = "Water Audio")
    bool DetectWaterNearby(const FVector& WorldLocation, float SearchRadius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Water Audio")
    void UpdateWaterAudio(bool bNearWater, float WaterDistance = 0.0f);

    // Audio Zone Management
    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void CreateAudioZoneAtLocation(const FVector& Location, EWorld_BiomeType BiomeType, float Radius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void RemoveAudioZonesInRadius(const FVector& Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void OptimizeAudioZones();

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugShowAudioZones();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void TestBiomeAudioTransition();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateLandscapeAudioSetup();

private:
    // Internal state
    float LastAudioUpdateTime;
    TArray<FVector> ActiveAudioZones;
    TMap<EWorld_BiomeType, float> BiomeBlendWeights;

    // Internal helper functions
    void InitializeDefaultBiomeSounds();
    void UpdateAudioComponentSettings();
    float CalculateBiomeBlendWeight(const FVector& Location, EWorld_BiomeType BiomeType);
    void CleanupInactiveAudioZones();
};