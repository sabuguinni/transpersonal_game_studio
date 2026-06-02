#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/RectLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "../SharedTypes.h"
#include "Light_UnderwaterCausticsSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_CausticsConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float WaveSpeed = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float WaveAmplitude = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float CausticsIntensity = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    FLinearColor WaterColor = FLinearColor(0.4f, 0.8f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float DepthFadeDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float CausticsScale = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caustics")
    float RefractiveIndex = 1.33f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLight_UnderwaterZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneExtent = FVector(2000.0f, 2000.0f, 500.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float WaterSurfaceHeight = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FLight_CausticsConfiguration CausticsConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float TurbidityLevel = 0.1f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_UnderwaterCausticsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_UnderwaterCausticsSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core underwater lighting functionality
    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    void InitializeUnderwaterZones();

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    void UpdateCausticsAnimation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    void SetUnderwaterZone(const FLight_UnderwaterZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    bool IsLocationUnderwater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    float GetWaterDepthAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    void CreateCausticsLightPattern(const FVector& SurfaceLocation, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Underwater Caustics")
    void UpdateUnderwaterAmbientLighting(float DepthFactor);

    // Caustics pattern generation
    UFUNCTION(BlueprintCallable, Category = "Caustics Pattern")
    void GenerateCausticsPattern();

    UFUNCTION(BlueprintCallable, Category = "Caustics Pattern")
    void UpdateCausticsIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Caustics Pattern")
    void SetCausticsWaveParameters(float Speed, float Amplitude);

    // Water surface interaction
    UFUNCTION(BlueprintCallable, Category = "Water Surface")
    void CalculateWaterSurfaceRefraction(const FVector& LightDirection, FVector& RefractedDirection);

    UFUNCTION(BlueprintCallable, Category = "Water Surface")
    void UpdateWaterSurfaceNormals();

    UFUNCTION(BlueprintCallable, Category = "Water Surface")
    void ApplyWaterColorGrading(float Depth);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCausticsLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCausticsVisibility(const FVector& ViewerLocation);

protected:
    // Underwater zones configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Underwater Zones")
    TArray<FLight_UnderwaterZone> UnderwaterZones;

    // Caustics lighting components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<class URectLightComponent*> CausticsLights;

    // Animation parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float CurrentWaveTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float CausticsAnimationSpeed = 1.0f;

    // Material instances for caustics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Materials")
    TArray<class UMaterialInstanceDynamic*> CausticsMaterials;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxCausticsDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActiveCausticsLights = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableCausticsLOD = true;

    // Water physics parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float WaterDensity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    float LightAbsorptionRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Physics")
    FLinearColor WaterAbsorptionColor = FLinearColor(0.2f, 0.4f, 0.8f, 1.0f);

private:
    // Internal caustics calculation
    void CalculateCausticsIntensity(const FVector& Position, float& OutIntensity);
    void UpdateCausticsLightPositions();
    void OptimizeCausticsPerformance();
    
    // Water surface simulation
    float CalculateWaterSurfaceHeight(const FVector& Position, float Time);
    FVector CalculateWaterSurfaceNormal(const FVector& Position, float Time);
    
    // Light ray tracing through water
    void TraceLightRayThroughWater(const FVector& StartPos, const FVector& Direction, FVector& OutHitPos, float& OutIntensity);
};