#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "EnvArt_WaterFeatureSystem.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_WaterFeatureType : uint8
{
    SmallPond       UMETA(DisplayName = "Small Pond"),
    LargePond       UMETA(DisplayName = "Large Pond"),
    Stream          UMETA(DisplayName = "Stream"),
    Waterfall       UMETA(DisplayName = "Waterfall"),
    Marsh           UMETA(DisplayName = "Marsh"),
    RiverBend       UMETA(DisplayName = "River Bend")
};

USTRUCT(BlueprintType)
struct FEnvArt_WaterFeatureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    EEnvArt_WaterFeatureType FeatureType = EEnvArt_WaterFeatureType::SmallPond;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    float WaterLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    float FlowSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    bool bHasMist = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    bool bHasRipples = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    bool bHasAmbientSound = true;

    FEnvArt_WaterFeatureConfig()
    {
        FeatureType = EEnvArt_WaterFeatureType::SmallPond;
        WaterLevel = 0.0f;
        FlowSpeed = 1.0f;
        bHasMist = true;
        bHasRipples = true;
        bHasAmbientSound = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_WaterFeatureSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_WaterFeatureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WaterSurfaceMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WaterBedMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* MistParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* RippleParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientWaterSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Feature")
    FEnvArt_WaterFeatureConfig WaterConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WaterSurfaceMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WaterBedMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void ConfigureWaterFeature(const FEnvArt_WaterFeatureConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void SetWaterLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void SetFlowSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void ToggleMist(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void ToggleRipples(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void ToggleAmbientSound(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void CreatePondConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void CreateStreamConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Water Feature")
    void CreateWaterfallConfiguration();

protected:
    void UpdateWaterFeatureVisuals();
    void ConfigureMistParticles();
    void ConfigureRippleParticles();
    void ConfigureAmbientSound();
    void SetupWaterMaterials();
};