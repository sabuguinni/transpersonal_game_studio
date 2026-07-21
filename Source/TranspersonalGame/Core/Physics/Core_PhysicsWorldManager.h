#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/WorldSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Core_PhysicsWorldManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WorldPhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float GlobalGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float AirDensity = 1.225f; // kg/m³ at sea level

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float WaterDensity = 1000.0f; // kg/m³

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float WindStrength = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    FVector WindDirection = FVector(1.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    float TemperatureKelvin = 288.15f; // 15°C

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    bool bEnableAdvancedPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    bool bEnableFluidSimulation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Physics")
    bool bEnableWeatherPhysics = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsRegion
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Region")
    FBox RegionBounds = FBox(ForceInit);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Region")
    FCore_WorldPhysicsSettings RegionSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Region")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Region")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Region")
    float Priority = 1.0f;
};

/**
 * Core Physics World Manager
 * Manages global physics settings, environmental physics, and world-scale physics simulation
 * Integrates with biome system and environmental conditions
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsWorldManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsWorldManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // World Physics Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void InitializeWorldPhysics();

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void UpdateWorldPhysics(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGlobalPhysicsSettings(const FCore_WorldPhysicsSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FCore_WorldPhysicsSettings GetGlobalPhysicsSettings() const { return GlobalPhysicsSettings; }

    // Regional Physics
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void RegisterPhysicsRegion(const FCore_PhysicsRegion& Region);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void UnregisterPhysicsRegion(const FBox& RegionBounds);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FCore_WorldPhysicsSettings GetPhysicsSettingsAtLocation(const FVector& WorldLocation) const;

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetWindParameters(float Strength, const FVector& Direction);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FVector GetWindForceAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetTemperature(float TemperatureInKelvin);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Gravity Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetGravityScale() const { return GlobalPhysicsSettings.GlobalGravityScale; }

    UFUNCTION(BlueprintPure, Category = "Physics World")
    FVector GetGravityVector() const;

    // Fluid Physics
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void EnableFluidSimulation(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Physics World")
    float GetFluidDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category = "Physics World")
    bool IsLocationUnderwater(const FVector& WorldLocation) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void SetPhysicsLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics World")
    void OptimizePhysicsForPerformance();

    // Debug and Validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics World")
    void ValidateWorldPhysics();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Physics World")
    void DebugDrawPhysicsRegions();

    UFUNCTION(BlueprintPure, Category = "Physics World")
    bool IsPhysicsSystemHealthy() const { return bIsSystemHealthy; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics World", meta = (AllowPrivateAccess = "true"))
    FCore_WorldPhysicsSettings GlobalPhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics World", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsRegion> PhysicsRegions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics World", meta = (AllowPrivateAccess = "true"))
    bool bIsSystemHealthy = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics World", meta = (AllowPrivateAccess = "true"))
    float PhysicsUpdateInterval = 0.016f; // 60 FPS

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics World", meta = (AllowPrivateAccess = "true"))
    int32 CurrentPhysicsLOD = 0;

private:
    // Internal state
    float LastPhysicsUpdate = 0.0f;
    bool bNeedsPhysicsUpdate = false;

    // Helper functions
    void UpdateGravitySettings();
    void UpdateEnvironmentalForces();
    void ProcessPhysicsRegions();
    FCore_PhysicsRegion* FindHighestPriorityRegion(const FVector& WorldLocation) const;
    void ApplyRegionalPhysics(const FCore_PhysicsRegion& Region);
};