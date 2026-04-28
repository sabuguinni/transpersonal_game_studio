#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "InteriorLightingSystem.generated.h"

UENUM(BlueprintType)
enum class EInteriorLightType : uint8
{
    Fire_Torch          UMETA(DisplayName = "Fire Torch"),
    Fire_Campfire       UMETA(DisplayName = "Fire Campfire"),
    Fire_Brazier        UMETA(DisplayName = "Fire Brazier"),
    Natural_Sunbeam     UMETA(DisplayName = "Natural Sunbeam"),
    Natural_Moonbeam    UMETA(DisplayName = "Natural Moonbeam"),
    Crystal_Glow        UMETA(DisplayName = "Crystal Glow"),
    Bioluminescent      UMETA(DisplayName = "Bioluminescent"),
    Ambient_Cave        UMETA(DisplayName = "Ambient Cave")
};

UENUM(BlueprintType)
enum class EInteriorZoneType : uint8
{
    Cave_Shallow        UMETA(DisplayName = "Shallow Cave"),
    Cave_Deep          UMETA(DisplayName = "Deep Cave"),
    Hut_Small          UMETA(DisplayName = "Small Hut"),
    Hut_Large          UMETA(DisplayName = "Large Hut"),
    Shelter_Lean       UMETA(DisplayName = "Lean-to Shelter"),
    Shelter_Rock       UMETA(DisplayName = "Rock Shelter"),
    Temple_Sacred      UMETA(DisplayName = "Sacred Temple"),
    Underground_Tunnel UMETA(DisplayName = "Underground Tunnel")
};

USTRUCT(BlueprintType)
struct FInteriorLightConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    EInteriorLightType LightType = EInteriorLightType::Fire_Torch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    FLinearColor LightColor = FLinearColor(1.0f, 0.7f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float Intensity = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
    float Falloff = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bFlickers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float FlickerIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    bool bCastShadows = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowBias = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    bool bHasParticleEffect = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VFX")
    class UParticleSystem* ParticleEffect = nullptr;
};

USTRUCT(BlueprintType)
struct FInteriorZoneSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EInteriorZoneType ZoneType = EInteriorZoneType::Cave_Shallow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    FLinearColor AmbientColor = FLinearColor(0.1f, 0.15f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient")
    float AmbientIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    bool bHasInteriorFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    float FogDensity = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fog")
    FLinearColor FogColor = FLinearColor(0.3f, 0.4f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float ExposureCompensation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Saturation = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    float Contrast = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    TArray<FInteriorLightConfiguration> RecommendedLights;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UInteriorLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteriorLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Zone management
    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void SetInteriorZone(EInteriorZoneType ZoneType, const FVector& ZoneCenter, float ZoneRadius);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void AddInteriorLight(EInteriorLightType LightType, const FVector& Location, const FRotator& Rotation = FRotator::ZeroRotator);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void RemoveInteriorLight(class ULightComponent* LightComponent);

    UFUNCTION(BlueprintCallable, Category = "Interior Lighting")
    void UpdateInteriorAmbient(float AmbientIntensity, const FLinearColor& AmbientColor);

    // Light animation
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetLightFlicker(class ULightComponent* LightComponent, bool bEnabled, float Speed = 2.0f, float Intensity = 0.3f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AnimateAllLights(bool bEnable);

    // Environmental effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetInteriorFog(bool bEnabled, float Density, const FLinearColor& Color);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateSunbeam(const FVector& StartLocation, const FVector& EndLocation, float Intensity = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void CreateMoonbeam(const FVector& StartLocation, const FVector& EndLocation, float Intensity = 1.0f);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeLightsForDistance(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLightingLOD(int32 LODLevel);

    // Material integration
    UFUNCTION(BlueprintCallable, Category = "Materials")
    void UpdateMaterialLighting(class UMaterialParameterCollection* MaterialCollection);

    // Events
    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnInteriorZoneChanged(EInteriorZoneType NewZone);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnLightAdded(class ULightComponent* NewLight);

protected:
    // Current zone configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    EInteriorZoneType CurrentZoneType = EInteriorZoneType::Cave_Shallow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float ZoneRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoOptimizeLights = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float OptimizationDistance = 2000.0f;

    // Zone presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EInteriorZoneType, FInteriorZoneSettings> ZonePresets;

    // Light type presets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presets")
    TMap<EInteriorLightType, FInteriorLightConfiguration> LightPresets;

    // Active lights
    UPROPERTY()
    TArray<class ULightComponent*> InteriorLights;

    UPROPERTY()
    TArray<class UPointLightComponent*> AnimatedLights;

    // Fog and atmosphere
    UPROPERTY()
    class AExponentialHeightFog* InteriorFog;

    UPROPERTY()
    class APostProcessVolume* InteriorPostProcess;

    // Material parameter collection for lighting
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* LightingMPC;

    // Animation state
    UPROPERTY()
    float AnimationTime = 0.0f;

    UPROPERTY()
    TMap<class ULightComponent*, float> LightFlickerStates;

private:
    void InitializeZonePresets();
    void InitializeLightPresets();
    void ApplyZoneSettings(const FInteriorZoneSettings& Settings);
    void CreateInteriorFog();
    void CreateInteriorPostProcess();
    void UpdateLightAnimation(float DeltaTime);
    void UpdateMaterialParameters();
    class ULightComponent* CreateLightComponent(EInteriorLightType LightType, const FVector& Location, const FRotator& Rotation);
    void ConfigureLightComponent(class ULightComponent* LightComp, const FInteriorLightConfiguration& Config);
    float CalculateFlickerValue(float Time, float Speed, float Intensity);
    void OptimizeLightComponent(class ULightComponent* LightComp, float Distance);
};