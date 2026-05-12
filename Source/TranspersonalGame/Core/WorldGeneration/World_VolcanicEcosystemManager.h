#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/TextureRenderTarget2D.h"
#include "SharedTypes.h"
#include "World_VolcanicEcosystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicVent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    float GasEmission;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Vent")
    bool bIsActive;

    FWorld_VolcanicVent()
    {
        Location = FVector::ZeroVector;
        Temperature = 800.0f;
        Activity = 0.5f;
        GasEmission = 0.3f;
        bIsActive = true;
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
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lava Flow")
    bool bIsFlowing;

    FWorld_LavaFlow()
    {
        FlowSpeed = 2.0f;
        Temperature = 1200.0f;
        Width = 5.0f;
        bIsFlowing = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicAudioZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FVector Center;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float RumblingIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float HissingSteamVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bPlayEruptionSounds;

    FWorld_VolcanicAudioZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        RumblingIntensity = 0.7f;
        HissingSteamVolume = 0.5f;
        bPlayEruptionSounds = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_VolcanicEcosystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_VolcanicEcosystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Volcanic System Management
    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void InitializeVolcanicEcosystem();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void UpdateVolcanicActivity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void CreateVolcanicVent(const FVector& Location, float Temperature, float Activity);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void CreateLavaFlow(const TArray<FVector>& FlowPath, float FlowSpeed);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void UpdateLavaFlows(float DeltaTime);

    // Audio Management
    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void CreateVolcanicAudioZone(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void UpdateVolcanicAudio(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void PlayEruptionSound(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Audio")
    void UpdateRumblingAudio(float Intensity);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void CreateGeothermalEffects(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void UpdateVolcanicParticles(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Effects")
    void CreateVolcanicLighting();

    // Ecosystem Integration
    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void SpawnVolcanicVegetation();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    void UpdateEcosystemTemperature(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    bool IsLocationInVolcanicZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Volcanic Ecosystem")
    float GetVolcanicTemperatureAtLocation(const FVector& Location) const;

protected:
    // Volcanic Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_VolcanicVent> VolcanicVents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_LavaFlow> LavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_VolcanicAudioZone> AudioZones;

    // Audio Components
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    TArray<UAudioComponent*> RumblingAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    TArray<UAudioComponent*> SteamAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* EruptionSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* RumblingSoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Audio")
    USoundCue* SteamHissingSoundCue;

    // Visual Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TArray<UParticleSystemComponent*> LavaParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TArray<UParticleSystemComponent*> SteamParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Effects")
    TArray<UStaticMeshComponent*> LavaFlowMeshes;

    // Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Materials")
    UMaterialInterface* LavaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Materials")
    UMaterialInterface* VolcanicRockMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Materials")
    UMaterialInterface* GeothermalSteamMaterial;

    // System Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Parameters")
    float BaseVolcanicActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Parameters")
    float VolcanicActivityVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Parameters")
    float MaxVolcanicTemperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Parameters")
    float VolcanicInfluenceRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Parameters")
    bool bVolcanicSystemActive;

private:
    // Internal state
    float VolcanicActivityTimer;
    float AudioUpdateTimer;
    float ParticleUpdateTimer;
    
    // Helper functions
    void UpdateVolcanicVentActivity(FWorld_VolcanicVent& Vent, float DeltaTime);
    void UpdateLavaFlowMovement(FWorld_LavaFlow& Flow, float DeltaTime);
    void UpdateAudioZoneIntensity(FWorld_VolcanicAudioZone& Zone, float DeltaTime);
    float CalculateDistanceToNearestVent(const FVector& Location) const;
    void CreateVolcanicParticleEffect(const FVector& Location, float Intensity);
};