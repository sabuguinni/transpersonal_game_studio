#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Arch_CretaceousStructuralIntegrator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralPlacement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Placement")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Placement")
    FRotator Rotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Placement")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Placement")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Placement")
    float IntegrationStrength;

    FArch_StructuralPlacement()
    {
        Location = FVector::ZeroVector;
        Rotation = FRotator::ZeroRotator;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Forest;
        IntegrationStrength = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_AtmosphericConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    FLinearColor LightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    float LightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    FRotator LightDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    float ShadowStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Configuration")
    bool bEnableVolumetricFog;

    FArch_AtmosphericConfiguration()
    {
        LightColor = FLinearColor(1.0f, 0.9f, 0.7f, 1.0f);
        LightIntensity = 3.0f;
        LightDirection = FRotator(-45.0f, 135.0f, 0.0f);
        ShadowStrength = 0.8f;
        bEnableVolumetricFog = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_CretaceousStructuralIntegrator : public AActor
{
    GENERATED_BODY()

public:
    AArch_CretaceousStructuralIntegrator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FoundationMeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructuralElementsComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integration")
    FArch_StructuralPlacement PlacementConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmospheric Integration")
    FArch_AtmosphericConfiguration AtmosphericSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integration")
    TArray<UStaticMesh*> CretaceousFoundationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Integration")
    TArray<UStaticMesh*> StructuralElementMeshes;

public:
    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void IntegrateWithBiome(EBiomeType TargetBiome, const FVector& BiomeLocation);

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void ConfigureAtmosphericLighting();

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void ApplyWeatheringEffects(float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void SetStructuralScale(const FVector& NewScale);

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    bool ValidateStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Structural Integration")
    void SpawnAtmosphericLighting(const FVector& LightLocation);

protected:
    UFUNCTION()
    void InitializeFoundationMesh();

    UFUNCTION()
    void InitializeStructuralElements();

    UFUNCTION()
    void ApplyBiomeSpecificMaterials(EBiomeType BiomeType);

    UFUNCTION()
    void ConfigureShadowSettings();

private:
    UPROPERTY()
    ADirectionalLight* AtmosphericLight;

    UPROPERTY()
    TArray<AActor*> SpawnedStructuralElements;

    float StructuralIntegrityValue;
    bool bIsAtmosphericLightingActive;
    bool bIsWeatheringApplied;
};