#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "EnvArt_BiomeAtmosphereSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphereProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-30.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    FLinearColor SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sky")
    float SkyLightIntensity = 1.0f;

    FEnvArt_AtmosphereProfile()
    {
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 3.0f;
        SunRotation = FRotator(-30.0f, 45.0f, 0.0f);
        FogColor = FLinearColor(0.7f, 0.8f, 1.0f, 1.0f);
        FogDensity = 0.02f;
        FogHeightFalloff = 0.2f;
        SkyLightColor = FLinearColor(0.5f, 0.7f, 1.0f, 1.0f);
        SkyLightIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_BiomeAtmosphereSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_BiomeAtmosphereSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UExponentialHeightFogComponent* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    FEnvArt_AtmosphereProfile SavanaProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    FEnvArt_AtmosphereProfile FlorestaProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    FEnvArt_AtmosphereProfile DesertoProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    FEnvArt_AtmosphereProfile PantanoProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere Profiles")
    FEnvArt_AtmosphereProfile MontanhaProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bEnableDayNightCycle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float DayDurationMinutes = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Detection")
    float BiomeTransitionRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Detection")
    float AtmosphereUpdateInterval = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereProfile(EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void StartDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void StopDayNightCycle();

    UFUNCTION(BlueprintCallable, Category = "Biome Detection")
    EBiomeType DetectCurrentBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void TransitionToProfile(const FEnvArt_AtmosphereProfile& TargetProfile, float TransitionDuration = 2.0f);

protected:
    UFUNCTION()
    void UpdateAtmosphere();

    UFUNCTION()
    void UpdateDayNightCycle();

    void ApplyAtmosphereProfile(const FEnvArt_AtmosphereProfile& Profile);
    void InterpolateAtmosphereProfiles(const FEnvArt_AtmosphereProfile& FromProfile, const FEnvArt_AtmosphereProfile& ToProfile, float Alpha);
    FEnvArt_AtmosphereProfile GetProfileForBiome(EBiomeType BiomeType);

private:
    FTimerHandle AtmosphereUpdateTimer;
    FTimerHandle DayNightTimer;
    
    FEnvArt_AtmosphereProfile CurrentProfile;
    FEnvArt_AtmosphereProfile TargetProfile;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    float TransitionSpeed = 1.0f;

    void InitializeAtmosphereProfiles();
    FVector GetBiomeCenter(EBiomeType BiomeType);
};