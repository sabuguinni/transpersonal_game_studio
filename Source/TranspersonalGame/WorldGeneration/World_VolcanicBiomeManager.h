#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/TriggerBox.h"
#include "Particles/ParticleSystemComponent.h"
#include "SharedTypes.h"
#include "World_VolcanicBiomeManager.generated.h"

/**
 * Manages volcanic biome features including lava flows, volcanic vents, obsidian formations,
 * and thermal springs. Provides dynamic volcanic activity and environmental hazards.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicBiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicBiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === VOLCANIC TERRAIN COMPONENTS ===
    
    /** Main volcanic landscape reference */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic Terrain")
    class ALandscape* VolcanicLandscape;

    /** Lava flow static mesh components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lava System")
    TArray<UStaticMeshComponent*> LavaFlowMeshes;

    /** Volcanic vent light components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Volcanic Vents")
    TArray<UPointLightComponent*> VolcanicVentLights;

    /** Obsidian rock formation meshes */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rock Formations")
    TArray<UStaticMeshComponent*> ObsidianRocks;

    /** Thermal spring water meshes */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Water Features")
    TArray<UStaticMeshComponent*> ThermalSprings;

    /** Volcanic ash particle system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Particle Effects")
    UParticleSystemComponent* VolcanicAshEmitter;

    /** Biome boundary trigger box */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome Management")
    class ATriggerBox* BiomeBoundary;

    // === VOLCANIC ACTIVITY SETTINGS ===

    /** Intensity of volcanic activity (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Activity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VolcanicActivityLevel;

    /** Temperature of lava flows in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Activity", meta = (ClampMin = "800.0", ClampMax = "1200.0"))
    float LavaTemperature;

    /** Rate of ash particle emission */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Activity", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float AshEmissionRate;

    /** Thermal spring temperature in Celsius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Features", meta = (ClampMin = "40.0", ClampMax = "90.0"))
    float ThermalSpringTemperature;

    // === ENVIRONMENTAL HAZARDS ===

    /** Damage per second from lava contact */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float LavaDamagePerSecond;

    /** Toxic gas damage from volcanic vents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float ToxicGasDamagePerSecond;

    /** Healing rate from thermal springs */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Hazards", meta = (ClampMin = "0.0", ClampMax = "20.0"))
    float ThermalSpringHealingRate;

public:
    // === VOLCANIC MANAGEMENT FUNCTIONS ===

    /** Initialize volcanic biome with all components */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Management")
    void InitializeVolcanicBiome();

    /** Update volcanic activity level dynamically */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Management")
    void SetVolcanicActivity(float NewActivityLevel);

    /** Trigger volcanic eruption event */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Management")
    void TriggerVolcanicEruption(float EruptionIntensity);

    /** Create new lava flow at specified location */
    UFUNCTION(BlueprintCallable, Category = "Lava System")
    void CreateLavaFlow(FVector StartLocation, FVector EndLocation, float FlowWidth);

    /** Add new volcanic vent with lighting and particles */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Vents")
    void AddVolcanicVent(FVector VentLocation, float VentIntensity);

    /** Generate obsidian rock formation cluster */
    UFUNCTION(BlueprintCallable, Category = "Rock Formations")
    void GenerateObsidianFormation(FVector CenterLocation, int32 RockCount, float SpreadRadius);

    /** Create thermal spring with healing properties */
    UFUNCTION(BlueprintCallable, Category = "Water Features")
    void CreateThermalSpring(FVector SpringLocation, float SpringRadius, float HealingPower);

    // === ENVIRONMENTAL INTERACTION ===

    /** Check if location is within volcanic biome */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome Management")
    bool IsLocationInVolcanicBiome(FVector TestLocation) const;

    /** Get environmental temperature at location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environmental Data")
    float GetEnvironmentalTemperature(FVector Location) const;

    /** Get volcanic hazard level at location */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environmental Data")
    float GetVolcanicHazardLevel(FVector Location) const;

    /** Check if location has toxic gas exposure */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Environmental Data")
    bool HasToxicGasExposure(FVector Location) const;

    // === RESOURCE MANAGEMENT ===

    /** Get obsidian resource nodes in area */
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    TArray<FVector> GetObsidianResourceLocations(FVector CenterLocation, float SearchRadius) const;

    /** Get thermal spring locations for healing */
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    TArray<FVector> GetThermalSpringLocations() const;

    /** Check if location is safe for player movement */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Safety Management")
    bool IsLocationSafeForMovement(FVector TestLocation) const;

private:
    // === INTERNAL SYSTEMS ===

    /** Update lava flow animations */
    void UpdateLavaFlows(float DeltaTime);

    /** Update volcanic vent lighting effects */
    void UpdateVolcanicVents(float DeltaTime);

    /** Update ash particle systems */
    void UpdateAshEmission(float DeltaTime);

    /** Update thermal spring effects */
    void UpdateThermalSprings(float DeltaTime);

    /** Calculate distance-based environmental effects */
    float CalculateProximityEffect(FVector TestLocation, FVector SourceLocation, float MaxDistance) const;

    /** Internal timer for volcanic activity cycles */
    float VolcanicActivityTimer;

    /** Current eruption state */
    bool bIsErupting;

    /** Eruption duration timer */
    float EruptionTimer;
};