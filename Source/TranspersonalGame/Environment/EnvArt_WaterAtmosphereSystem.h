#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "EnvArt_WaterAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WaterEffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    float MistDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    float MistHeight = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    float SoundVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    float EffectRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    bool bEnableRipples = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Effects")
    bool bEnableFoam = true;

    FEnvArt_WaterEffectSettings()
    {
        MistDensity = 0.5f;
        MistHeight = 200.0f;
        SoundVolume = 0.7f;
        EffectRadius = 1000.0f;
        bEnableRipples = true;
        bEnableFoam = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WaterBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Biome")
    FVector WaterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Biome")
    FEnvArt_WaterEffectSettings EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Biome")
    float WaterTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Biome")
    float FlowRate = 1.0f;

    FEnvArt_WaterBiomeData()
    {
        BiomeType = EBiomeType::Forest;
        WaterLocation = FVector::ZeroVector;
        WaterTemperature = 20.0f;
        FlowRate = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WaterAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_WaterAtmosphereSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* MistParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* FoamParticleComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* WaterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WaterSurfaceComponent;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Atmosphere")
    TArray<FEnvArt_WaterBiomeData> BiomeWaterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Atmosphere")
    FEnvArt_WaterEffectSettings GlobalEffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Atmosphere")
    float UpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Atmosphere")
    bool bEnableAtmosphericEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Atmosphere")
    bool bEnableDynamicMist = true;

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void InitializeWaterEffects();

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void UpdateAtmosphericEffects(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void SetBiomeWaterEffect(EBiomeType BiomeType, const FEnvArt_WaterEffectSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    FEnvArt_WaterEffectSettings GetBiomeWaterEffect(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void CreateMistEffect(const FVector& Location, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void CreateWaterSound(const FVector& Location, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void UpdateWaterTemperature(float NewTemperature);

    UFUNCTION(BlueprintCallable, Category = "Water Atmosphere")
    void SetFlowRate(float NewFlowRate);

private:
    float LastUpdateTime = 0.0f;
    TArray<class UParticleSystemComponent*> ActiveMistEffects;
    TArray<class UAudioComponent*> ActiveWaterSounds;

    void UpdateMistEffects(float DeltaTime);
    void UpdateWaterSounds(float DeltaTime);
    void CleanupInactiveEffects();
};