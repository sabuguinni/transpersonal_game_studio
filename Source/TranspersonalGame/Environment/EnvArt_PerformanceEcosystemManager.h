#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerVolume.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "SharedTypes.h"
#include "EnvArt_PerformanceEcosystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EcosystemZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxVegetationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    FEnvArt_EcosystemZone()
    {
        ZoneName = TEXT("DefaultZone");
        Position = FVector::ZeroVector;
        Radius = 1000.0f;
        BiomeType = EBiomeType::Forest;
        MaxVegetationCount = 50;
        LODDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_VegetationCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector ClusterPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<AActor*> VegetationActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float DistanceToPlayer;

    FEnvArt_VegetationCluster()
    {
        ClusterPosition = FVector::ZeroVector;
        LODLevel = 1.0f;
        bIsVisible = true;
        DistanceToPlayer = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FString EffectName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FVector EffectPosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    UParticleSystemComponent* ParticleComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDrawDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDistanceCulling;

    FEnvArt_AtmosphericEffect()
    {
        EffectName = TEXT("DefaultEffect");
        EffectPosition = FVector::ZeroVector;
        ParticleComponent = nullptr;
        MaxDrawDistance = 3000.0f;
        bUseDistanceCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_PerformanceEcosystemManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_PerformanceEcosystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === ECOSYSTEM MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ecosystem Zones")
    TArray<FEnvArt_EcosystemZone> EcosystemZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TArray<FEnvArt_VegetationCluster> VegetationClusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    TArray<FEnvArt_AtmosphericEffect> AtmosphericEffects;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxRenderDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODTransitionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    // === LIGHTING OPTIMIZATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float ShadowDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    bool bUseDynamicShadows;

public:
    // === ECOSYSTEM FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void InitializeEcosystemZones();

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void CreateVegetationCluster(const FVector& Position, EBiomeType BiomeType, int32 VegetationCount);

    UFUNCTION(BlueprintCallable, Category = "Ecosystem")
    void UpdateEcosystemPerformance();

    // === VEGETATION MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void SpawnVegetationInZone(const FEnvArt_EcosystemZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void OptimizeVegetationLOD(float PlayerDistance);

    UFUNCTION(BlueprintCallable, Category = "Vegetation")
    void CullDistantVegetation();

    // === ATMOSPHERIC EFFECTS ===
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateAtmosphericEffect(const FString& EffectName, const FVector& Position, const FString& EffectType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphericLOD();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void OptimizeLightingPerformance();

    // === PERFORMANCE MONITORING ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustQualitySettings(float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void GeneratePerformanceReport();

    // === ENVIRONMENTAL STORYTELLING ===
    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void PlaceStorytellingProps();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void CreateAmbientSoundZones();

    UFUNCTION(BlueprintCallable, Category = "Storytelling")
    void SetupEnvironmentalNarrative();

private:
    // === INTERNAL SYSTEMS ===
    void UpdatePerformanceMetrics();
    void ProcessLODTransitions();
    void OptimizeRenderingPipeline();
    void ManageMemoryUsage();

    // === PERFORMANCE TRACKING ===
    float LastPerformanceUpdate;
    float CurrentFPS;
    int32 ActiveVegetationCount;
    int32 ActiveParticleCount;
    float MemoryUsage;
};