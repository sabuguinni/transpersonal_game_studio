#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/LightComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "SharedTypes.h"
#include "Light_BiomeLightingController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_BiomeLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor SunColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float SunIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float FogDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    FLinearColor FogColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float AtmosphericPerspective;

    FLight_BiomeLightingProfile()
    {
        AmbientColor = FLinearColor(0.1f, 0.1f, 0.15f, 1.0f);
        SunColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        SunIntensity = 8.0f;
        FogDensity = 0.02f;
        FogColor = FLinearColor(0.8f, 0.85f, 0.9f, 1.0f);
        AtmosphericPerspective = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_BiomeLightingController : public AActor
{
    GENERATED_BODY()

public:
    ALight_BiomeLightingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    TMap<EBiomeType, FLight_BiomeLightingProfile> BiomeLightingProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float TransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Lighting")
    float BiomeDetectionRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Lighting")
    EBiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Lighting")
    EBiomeType TargetBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Biome Lighting")
    float TransitionProgress;

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void SetBiomeLighting(EBiomeType BiomeType, bool bInstant = false);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    EBiomeType DetectCurrentBiome(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    void ApplyLightingProfile(const FLight_BiomeLightingProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Biome Lighting")
    FLight_BiomeLightingProfile GetCurrentLightingProfile() const;

protected:
    UFUNCTION()
    void InitializeBiomeLightingProfiles();

    UFUNCTION()
    void UpdateLightingTransition(float DeltaTime);

    UFUNCTION()
    void ApplyDirectionalLighting(const FLight_BiomeLightingProfile& Profile);

    UFUNCTION()
    void ApplyAtmosphericLighting(const FLight_BiomeLightingProfile& Profile);

private:
    UPROPERTY()
    ADirectionalLight* MainDirectionalLight;

    UPROPERTY()
    TArray<APointLight*> AtmosphericLights;

    FLight_BiomeLightingProfile CurrentProfile;
    FLight_BiomeLightingProfile TargetProfile;
};