#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "World_VolcanicBiomeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Intensity; // 0.0 to 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_VolcanicFeatureType FeatureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    bool bIsActive;

    FWorld_VolcanicFeature()
    {
        Location = FVector::ZeroVector;
        Intensity = 0.5f;
        FeatureType = EWorld_VolcanicFeatureType::SteamVent;
        Radius = 500.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LavaFlowData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    TArray<FVector> FlowPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float FlowSpeed; // Units per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Temperature; // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    bool bIsFlowing;

    FWorld_LavaFlowData()
    {
        FlowSpeed = 100.0f;
        Temperature = 1200.0f;
        Width = 200.0f;
        bIsFlowing = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_VolcanicBiomeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_VolcanicBiomeSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === VOLCANIC FEATURE MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void GenerateVolcanicFeatures(const FVector& CenterLocation, float BiomeRadius);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnSteamVent(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnLavaPool(const FVector& Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateLavaFlow(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnVolcanicRock(const FVector& Location, float Scale);

    // === ENVIRONMENTAL EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicActivity(float ActivityLevel);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnAshParticles(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateHeatDistortion(float DistortionStrength);

    // === VEGETATION SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnVolcanicVegetation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void PlaceFerns(const FVector& CenterLocation, float Radius, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void PlaceCycads(const FVector& CenterLocation, float Radius, int32 Count);

    // === AUDIO SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void PlayLavaBubbleSound(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void PlaySteamHissSound(const FVector& Location);

    // === TEMPERATURE SYSTEM ===
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsLocationTooHot(const FVector& Location, float Threshold = 80.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateTemperatureField();

protected:
    // === VOLCANIC FEATURES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    TArray<FWorld_VolcanicFeature> VolcanicFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    TArray<FWorld_LavaFlowData> LavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    float BaseTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Features")
    float VolcanicActivityLevel; // 0.0 to 1.0

    // === MESHES AND MATERIALS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* SteamVentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* LavaPoolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* VolcanicRockMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* FernMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* CycadMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* LavaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* VolcanicRockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UMaterialInterface* AshMaterial;

    // === PARTICLE SYSTEMS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* SteamParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* AshParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* LavaBubbleParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* HeatDistortionParticles;

    // === AUDIO ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* VolcanicAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* LavaBubbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* SteamHissSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class UAudioComponent* AmbientAudioComponent;

    // === SPAWNED COMPONENTS ===
    UPROPERTY()
    TArray<class UStaticMeshComponent*> SpawnedMeshComponents;

    UPROPERTY()
    TArray<class UParticleSystemComponent*> SpawnedParticleComponents;

    UPROPERTY()
    TArray<class UAudioComponent*> SpawnedAudioComponents;

private:
    // === INTERNAL HELPERS ===
    void InitializeVolcanicAssets();
    void CleanupSpawnedComponents();
    FVector FindSuitableVolcanicLocation(const FVector& CenterLocation, float SearchRadius) const;
    bool IsValidVolcanicSpawnLocation(const FVector& Location) const;
    float CalculateHeatIntensity(const FVector& Location) const;
    void UpdateLavaFlowAnimation(float DeltaTime);
    void UpdateParticleEffects(float DeltaTime);
};