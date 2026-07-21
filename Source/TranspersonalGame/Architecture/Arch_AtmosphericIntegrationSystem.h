#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_AtmosphericIntegrationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AtmosphericStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    FVector StructureLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    FRotator StructureRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    float AtmosphericDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    FLinearColor AmbientLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    bool bHasVolumetricFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Structure")
    float FogDensity;

    FArch_AtmosphericStructureData()
    {
        StructureLocation = FVector::ZeroVector;
        StructureRotation = FRotator::ZeroRotator;
        BiomeType = EBiomeType::Forest;
        AtmosphericDensity = 1.0f;
        AmbientLightColor = FLinearColor::White;
        bHasVolumetricFog = true;
        FogDensity = 0.02f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_ShelterFoundationData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    FVector BaseLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float FoundationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    ESurfaceType SurfaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    float StructuralIntegrity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    bool bNaturalFormation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foundation")
    TArray<FVector> SupportPoints;

    FArch_ShelterFoundationData()
    {
        BaseLocation = FVector::ZeroVector;
        FoundationRadius = 500.0f;
        SurfaceType = ESurfaceType::Rock;
        StructuralIntegrity = 100.0f;
        bNaturalFormation = true;
        SupportPoints.Empty();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_AtmosphericIntegrationSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_AtmosphericIntegrationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AtmosphericParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    FArch_AtmosphericStructureData AtmosphericData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    FArch_ShelterFoundationData FoundationData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    float LightingIntensityMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    bool bAdaptToTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    float WeatherResponsiveness;

public:
    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    void InitializeAtmosphericStructure(const FArch_AtmosphericStructureData& InAtmosphericData);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    void SetupShelterFoundation(const FArch_ShelterFoundationData& InFoundationData);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    void UpdateAtmosphericEffects(float TimeOfDay, float WeatherIntensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    void AdaptToEnvironmentalLighting(const FLinearColor& AmbientColor, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    bool ValidateStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Atmospheric Integration")
    FVector GetOptimalShelterLocation(const FVector& PlayerLocation, float SearchRadius) const;

protected:
    UFUNCTION()
    void OnAtmosphericConditionsChanged();

    UFUNCTION()
    void UpdateParticleEffects();

    UFUNCTION()
    void AdjustAmbientAudio();

private:
    float CurrentTimeOfDay;
    float CurrentWeatherIntensity;
    bool bIsInitialized;

    void SetupDefaultFoundation();
    void ConfigureAtmosphericParticles();
    void InitializeAmbientAudio();
};