#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyAtmosphere.h"
#include "SharedTypes.h"
#include "EnvArt_BiomeAtmosphere.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_AtmosphericProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float SunIntensity = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FRotator SunRotation = FRotator(-15.0f, 45.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    float AtmosphereHaziness = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    FLinearColor GroundAlbedo = FLinearColor(0.3f, 0.25f, 0.2f, 1.0f);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEnvArt_BiomeAtmosphere : public UActorComponent
{
    GENERATED_BODY()

public:
    UEnvArt_BiomeAtmosphere();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Profiles")
    TMap<EBiomeType, FEnvArt_AtmosphericProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "References")
    ASkyAtmosphere* SkyAtmosphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionSpeed = 2.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EBiomeType CurrentBiome = EBiomeType::Savana;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetBiomeAtmosphere(EBiomeType NewBiome, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void InitializeAtmosphericActors();

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void ApplyAtmosphericProfile(const FEnvArt_AtmosphericProfile& Profile, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FEnvArt_AtmosphericProfile GetBiomeProfile(EBiomeType Biome) const;

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetTimeOfDay(float TimeNormalized);

    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void CreateDefaultBiomeProfiles();

private:
    void InterpolateAtmosphere(const FEnvArt_AtmosphericProfile& From, const FEnvArt_AtmosphericProfile& To, float Alpha);
    
    FEnvArt_AtmosphericProfile TargetProfile;
    FEnvArt_AtmosphericProfile CurrentProfile;
    float TransitionProgress = 0.0f;
};