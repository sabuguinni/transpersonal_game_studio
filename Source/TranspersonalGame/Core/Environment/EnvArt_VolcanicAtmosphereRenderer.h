#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/LightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "SharedTypes.h"
#include "EnvArt_VolcanicAtmosphereRenderer.generated.h"

/**
 * Volcanic Atmosphere Renderer Component
 * Handles atmospheric effects specific to volcanic environments:
 * - Ash particle systems with wind interaction
 * - Heat distortion effects from geothermal vents
 * - Sulfur gas atmospheric coloring
 * - Dynamic lighting from lava glow
 * - Steam and vapor effects from hot springs
 */
UCLASS(ClassGroup=(Environment), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_VolcanicAtmosphereRenderer : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_VolcanicAtmosphereRenderer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === VOLCANIC ATMOSPHERE PARAMETERS ===
    
    /** Intensity of volcanic ash in the atmosphere (0.0 to 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AshDensity = 0.3f;
    
    /** Color tint for volcanic atmosphere (sulfur yellows, ash grays) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere")
    FLinearColor VolcanicAtmosphereTint = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
    
    /** Intensity of heat distortion effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float HeatDistortionIntensity = 0.8f;
    
    /** Range of volcanic atmospheric effects in meters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "100.0", ClampMax = "10000.0"))
    float AtmosphericRange = 2000.0f;
    
    /** Wind strength affecting ash particles */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "5.0"))
    float WindStrength = 1.2f;
    
    /** Temperature increase from volcanic activity (affects particle behavior) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Atmosphere", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float TemperatureIncrease = 15.0f;

    // === PARTICLE SYSTEM REFERENCES ===
    
    /** Ash particle system for atmospheric effects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    class UParticleSystem* AshParticleSystem;
    
    /** Steam/vapor particle system for geothermal vents */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    class UParticleSystem* SteamParticleSystem;
    
    /** Sulfur gas particle system for volcanic emissions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    class UParticleSystem* SulfurGasParticleSystem;

    // === MATERIAL PARAMETER COLLECTION ===
    
    /** Material parameter collection for global volcanic atmosphere parameters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialParameterCollection* VolcanicAtmosphereMPC;

    // === COMPONENT REFERENCES ===
    
    /** Active ash particle components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UParticleSystemComponent*> AshParticleComponents;
    
    /** Active steam particle components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UParticleSystemComponent*> SteamParticleComponents;
    
    /** Active sulfur gas particle components */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class UParticleSystemComponent*> SulfurGasComponents;

    // === ATMOSPHERIC CONTROL FUNCTIONS ===
    
    /** Initialize volcanic atmosphere effects */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void InitializeVolcanicAtmosphere();
    
    /** Update atmospheric parameters based on volcanic activity level */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateAtmosphericIntensity(float VolcanicActivity);
    
    /** Spawn ash particle effects at specified locations */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SpawnAshEffects(const TArray<FVector>& SpawnLocations);
    
    /** Spawn steam effects at geothermal vent locations */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void SpawnSteamEffects(const TArray<FVector>& VentLocations);
    
    /** Update wind effects on atmospheric particles */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateWindEffects(FVector WindDirection, float WindSpeed);
    
    /** Set global volcanic atmosphere parameters via Material Parameter Collection */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void UpdateGlobalAtmosphericParameters();
    
    /** Clean up atmospheric effects when leaving volcanic area */
    UFUNCTION(BlueprintCallable, Category = "Volcanic Atmosphere")
    void CleanupVolcanicEffects();

    // === ATMOSPHERIC QUERIES ===
    
    /** Get current atmospheric visibility (affected by ash density) */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Volcanic Atmosphere")
    float GetAtmosphericVisibility() const;
    
    /** Get current temperature increase from volcanic activity */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Volcanic Atmosphere")
    float GetVolcanicTemperature() const;
    
    /** Check if location is within volcanic atmospheric influence */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Volcanic Atmosphere")
    bool IsLocationInVolcanicAtmosphere(FVector Location) const;

private:
    // === INTERNAL STATE ===
    
    /** Current volcanic activity level (0.0 to 1.0) */
    float CurrentVolcanicActivity = 0.5f;
    
    /** Timer for atmospheric effect updates */
    float AtmosphericUpdateTimer = 0.0f;
    
    /** Cached world reference */
    UWorld* CachedWorld = nullptr;
    
    /** Internal update functions */
    void UpdateParticleParameters();
    void UpdateMaterialParameters();
    void CleanupInactiveParticles();
};