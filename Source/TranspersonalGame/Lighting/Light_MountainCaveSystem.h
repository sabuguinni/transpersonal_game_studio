#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Light_MountainCaveSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_CaveLightingType : uint8
{
    Ambient         UMETA(DisplayName = "Ambient Cave Light"),
    Entrance        UMETA(DisplayName = "Cave Entrance Light"),
    Interior        UMETA(DisplayName = "Deep Interior Light"),
    Crystal         UMETA(DisplayName = "Crystal Formation Light"),
    Fire            UMETA(DisplayName = "Fire/Torch Light")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CaveLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    ELight_CaveLightingType LightType = ELight_CaveLightingType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float Intensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor LightColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bCastShadows = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bFlickerEffect = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FlickerSpeed = 2.0f;

    FLight_CaveLightConfig()
    {
        LightType = ELight_CaveLightingType::Ambient;
        Intensity = 2.0f;
        LightColor = FLinearColor(0.8f, 0.7f, 0.5f, 1.0f);
        AttenuationRadius = 800.0f;
        bCastShadows = false;
        bFlickerEffect = false;
        FlickerSpeed = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_MountainCaveSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_MountainCaveSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* PrimaryLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* LightSourceMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLight_CaveLightConfig LightConfiguration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    TArray<FLight_CaveLightConfig> AdditionalLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bEnableVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float FogHeightFalloff = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    FLinearColor FogInscatteringColor = FLinearColor(0.7f, 0.8f, 0.9f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bEnableDepthBasedLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float CaveDepthFactor = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    bool bEnableTemperatureBasedLighting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Lighting")
    float CaveTemperature = 15.0f;

private:
    UPROPERTY()
    TArray<class UPointLightComponent*> SecondaryLights;

    UPROPERTY()
    class AExponentialHeightFog* CaveFogActor;

    float FlickerTimer = 0.0f;
    float BaseIntensity = 2.0f;
    bool bFlickerDirection = true;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetupCaveLighting(const FLight_CaveLightConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void AddSecondaryLight(const FLight_CaveLightConfig& Config, const FVector& RelativeLocation);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateLightingForDepth(float CaveDepth);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void UpdateLightingForTemperature(float Temperature);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetFlickerEffect(bool bEnable, float Speed = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void CreateVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void RemoveVolumetricFog();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting", CallInEditor)
    void PreviewLightingSetup();

    UFUNCTION(BlueprintCallable, Category = "Cave Lighting")
    void SetLightingPreset(ELight_CaveLightingType PresetType);

protected:
    void UpdateFlickerEffect(float DeltaTime);
    void ApplyDepthBasedDimming(float Depth);
    void ApplyTemperatureColorShift(float Temperature);
    FLinearColor CalculateTemperatureColor(float Temperature);
    void CreateSecondaryLightComponent(const FLight_CaveLightConfig& Config, const FVector& Location);
};