#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "World_VolcanicBiome.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_VolcanicFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Intensity; // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    EWorld_VolcanicType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Feature")
    bool bIsActive;

    FWorld_VolcanicFeature()
    {
        Location = FVector::ZeroVector;
        Intensity = 0.5f;
        Type = EWorld_VolcanicType::SteamVent;
        Radius = 500.0f;
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
    bool bIsCooling;

    FWorld_LavaFlow()
    {
        FlowSpeed = 2.0f;
        Temperature = 1200.0f;
        Width = 100.0f;
        bIsCooling = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_VolcanicBiome : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_VolcanicBiome();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Volcanic Features
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_VolcanicFeature> VolcanicFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    TArray<FWorld_LavaFlow> LavaFlows;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float GlobalVolcanicActivity; // 0-1 scale

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float EruptionProbability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic System")
    float AmbientTemperature;

    // Visual Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    class UParticleSystem* SteamVentEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    class UParticleSystem* LavaFlowEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    class UParticleSystem* AshCloudEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    class UMaterialInterface* LavaMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    class UMaterialInterface* ObsidianMaterial;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* VolcanicAmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* EruptionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundCue* LavaFlowSound;

    // Generation Functions
    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void GenerateVolcanicFeatures(int32 NumFeatures = 10);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void CreateLavaFlow(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void SpawnVolcanicRocks(FVector Center, float Radius, int32 Count = 20);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Generation")
    void CreateObsidianFormations(FVector Location, float Size = 300.0f);

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void UpdateVolcanicActivity(float NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    void TriggerEruption(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    float GetTemperatureAtLocation(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environmental Effects")
    bool IsLocationInLavaZone(FVector Location) const;

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Utility")
    void ClearAllVolcanicFeatures();

    UFUNCTION(BlueprintCallable, Category = "Utility")
    FVector GetNearestVolcanicFeature(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Utility")
    TArray<FVector> GetSafeZones(float MinDistance = 1000.0f) const;

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void RegenerateVolcanicBiome();

    UFUNCTION(CallInEditor, Category = "Editor Tools")
    void PreviewVolcanicEffects();

private:
    // Internal state
    float LastEruptionTime;
    TArray<class UParticleSystemComponent*> ActiveEffects;
    TArray<class UAudioComponent*> ActiveAudioSources;

    // Helper functions
    void UpdateLavaFlows(float DeltaTime);
    void UpdateVolcanicEffects();
    void SpawnVolcanicVegetation();
    FVector CalculateLavaFlowDirection(FVector CurrentPos) const;
    void CreateVolcanicTerrain(FVector Center, float Radius);
};