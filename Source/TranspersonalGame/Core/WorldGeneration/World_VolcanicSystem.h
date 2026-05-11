#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Engine/DirectionalLight.h"
#include "Landscape/Landscape.h"
#include "SharedTypes.h"
#include "World_VolcanicSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicVent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float Temperature;

    FWorld_VolcanicVent()
    {
        Location = FVector::ZeroVector;
        Intensity = 1.0f;
        Radius = 500.0f;
        bIsActive = true;
        Temperature = 1200.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LavaFlow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    TArray<FVector> FlowPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    bool bIsActive;

    FWorld_LavaFlow()
    {
        FlowSpeed = 50.0f;
        Width = 200.0f;
        Temperature = 1000.0f;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_VolcanicSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_VolcanicSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core volcanic system properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_VolcanicVent> VolcanicVents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_LavaFlow> LavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float SystemIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float AmbientTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    bool bSystemActive;

    // Visual components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* VolcanoMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SmokeParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* LavaParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VolcanicAudio;

    // Volcanic activity functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateVolcanicVent(FVector Location, float Intensity = 1.0f, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateLavaFlow(const TArray<FVector>& FlowPath, float Speed = 50.0f, float Width = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void ActivateVolcanicSystem();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void DeactivateVolcanicSystem();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicActivity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnVolcanicEffects();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    float GetTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    bool IsLocationInVolcanicZone(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void ModifyTerrainForVolcanic();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateVolcanicLandscape();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateLavaFlows(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void SpawnVolcanicRocks();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void CreateGeothermalVents();

    UFUNCTION(BlueprintCallable, Category = "Volcanic System")
    void UpdateVolcanicAudio();

private:
    // Internal tracking
    float LastUpdateTime;
    float VolcanicCycleTime;
    bool bEruptionInProgress;
    
    // Helper functions
    void InitializeVolcanicComponents();
    void UpdateVolcanicVisuals(float DeltaTime);
    void ProcessLavaFlow(FWorld_LavaFlow& LavaFlow, float DeltaTime);
    void UpdateVolcanicVent(FWorld_VolcanicVent& Vent, float DeltaTime);
    FVector CalculateLavaFlowDirection(const FWorld_LavaFlow& LavaFlow, int32 PathIndex) const;
    float CalculateVolcanicIntensity() const;
};