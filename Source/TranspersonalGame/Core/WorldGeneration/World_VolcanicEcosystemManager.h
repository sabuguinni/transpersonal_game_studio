#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "World_VolcanicEcosystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zone")
    float TemperatureIncrease;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zone")
    float ToxicGasLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zone")
    bool bIsActiveVolcano;

    FWorld_VolcanicZoneData()
    {
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        TemperatureIncrease = 25.0f;
        ToxicGasLevel = 0.3f;
        bIsActiveVolcano = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_GeothermalFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    EWorld_GeothermalType FeatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float EffectRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    bool bIsHazardous;

    FWorld_GeothermalFeature()
    {
        Location = FVector::ZeroVector;
        FeatureType = EWorld_GeothermalType::HotSpring;
        Intensity = 1.0f;
        EffectRadius = 1000.0f;
        bIsHazardous = false;
    }
};

/**
 * Manages volcanic ecosystems and geothermal features in the Cretaceous world
 * Handles temperature zones, toxic gas areas, geothermal vents, and volcanic activity
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicEcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicEcosystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === VOLCANIC ZONE MANAGEMENT ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zones")
    TArray<FWorld_VolcanicZoneData> VolcanicZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zones")
    float VolcanicActivityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zones")
    float EruptionProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Zones")
    bool bVolcanicWeatherEnabled;

    // === GEOTHERMAL FEATURES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    TArray<FWorld_GeothermalFeature> GeothermalFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    int32 MaxHotSprings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    int32 MaxGeyserFields;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float GeothermalDensity;

    // === ENVIRONMENTAL EFFECTS ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UParticleSystemComponent* VolcanicSmokeComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* VolcanicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* VolcanicMeshComponent;

    // === ECOSYSTEM FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void InitializeVolcanicZones();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void SpawnGeothermalFeatures();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void UpdateVolcanicActivity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    bool IsLocationInVolcanicZone(const FVector& Location, float& TemperatureModifier) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void TriggerVolcanicEruption(int32 ZoneIndex);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void CreateGeothermalVent(const FVector& Location, EWorld_GeothermalType VentType);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    float GetToxicGasLevelAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    TArray<FVector> GetNearbyGeothermalFeatures(const FVector& Location, float SearchRadius) const;

    // === AUDIO AND EFFECTS ===
    
    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void PlayVolcanicAmbientSound();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void StartVolcanicParticleEffects();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void StopVolcanicEffects();

protected:
    // === INTERNAL SYSTEMS ===
    
    UPROPERTY()
    float VolcanicUpdateTimer;

    UPROPERTY()
    bool bEffectsActive;

    UPROPERTY()
    TArray<class AActor*> SpawnedGeothermalActors;

    // === HELPER FUNCTIONS ===
    
    void UpdateVolcanicZoneEffects();
    void ProcessGeothermalActivity();
    void HandleVolcanicWeatherEffects();
    FVector FindOptimalGeothermalLocation() const;
    bool IsValidGeothermalLocation(const FVector& Location) const;
};