#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystem.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_GeothermalSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_GeothermalVentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal")
    bool bIsActive;

    FWorld_GeothermalVentData()
    {
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Temperature = 80.0f;
        Radius = 500.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_HotSpringData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs")
    float WaterTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs")
    float PoolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs")
    int32 PoolDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs")
    bool bHasSteam;

    FWorld_HotSpringData()
    {
        Location = FVector::ZeroVector;
        WaterTemperature = 45.0f;
        PoolRadius = 300.0f;
        PoolDepth = 200;
        bHasSteam = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_GeothermalSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_GeothermalSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Geothermal vents management
    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void GenerateGeothermalVents(int32 NumVents, float MinDistance = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void CreateGeothermalVent(const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Geothermal")
    void RemoveGeothermalVent(int32 VentIndex);

    // Hot springs management
    UFUNCTION(BlueprintCallable, Category = "Hot Springs")
    void GenerateHotSprings(int32 NumSprings, float MinDistance = 1500.0f);

    UFUNCTION(BlueprintCallable, Category = "Hot Springs")
    void CreateHotSpring(const FVector& Location, float Temperature = 45.0f);

    UFUNCTION(BlueprintCallable, Category = "Hot Springs")
    void RemoveHotSpring(int32 SpringIndex);

    // Geothermal effects
    UFUNCTION(BlueprintCallable, Category = "Geothermal Effects")
    void UpdateGeothermalEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Geothermal Effects")
    void SpawnSteamEffects(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Geothermal Effects")
    void UpdateHotSpringEffects(int32 SpringIndex);

    // Temperature and environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool IsLocationNearGeothermalActivity(const FVector& Location, float CheckRadius = 1000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ApplyGeothermalEnvironmentalEffects();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearAllGeothermalFeatures();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetActiveVentCount() const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    int32 GetActiveSpringCount() const;

protected:
    // Geothermal vents data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Geothermal Data")
    TArray<FWorld_GeothermalVentData> GeothermalVents;

    // Hot springs data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hot Springs Data")
    TArray<FWorld_HotSpringData> HotSprings;

    // Generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float WorldBounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinVentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxVentIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinSpringTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MaxSpringTemperature;

    // Visual and audio assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UStaticMesh* VentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UStaticMesh* HotSpringMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UParticleSystem* SteamParticleSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    USoundCue* GeothermalAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    USoundCue* HotSpringBubblingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UMaterialInterface* HotWaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    UMaterialInterface* GeothermalRockMaterial;

    // Runtime tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedVentActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<AActor*> SpawnedSpringActors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UParticleSystemComponent*> ActiveSteamEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    TArray<UAudioComponent*> ActiveAudioSources;

private:
    // Helper functions
    FVector FindValidGeothermalLocation(float MinDistance) const;
    bool IsLocationValidForGeothermal(const FVector& Location, float MinDistance) const;
    void CleanupGeothermalActors();
    void InitializeDefaultAssets();
};