#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Light_VolcanicLightingSystem.generated.h"

class UNiagaraComponent;
class UMaterialParameterCollection;

UENUM(BlueprintType)
enum class ELight_VolcanicLightState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    LowGlow         UMETA(DisplayName = "Low Glow"),
    ActiveGlow      UMETA(DisplayName = "Active Glow"),
    IntenseGlow     UMETA(DisplayName = "Intense Glow"),
    Erupting        UMETA(DisplayName = "Erupting")
};

USTRUCT(BlueprintType)
struct FLight_VolcanicLightConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float BaseIntensity = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float MaxIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    FLinearColor LavaColor = FLinearColor(1.0f, 0.3f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float HeatShimmerIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    float VolcanicAshDensity = 0.3f;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULight_VolcanicLightingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULight_VolcanicLightingSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void SetVolcanicState(ELight_VolcanicLightState NewState);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void ActivateLavaGlow(float Intensity = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void DeactivateLavaGlow();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void CreateHeatShimmerEffect(FVector Location, float Radius = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void SpawnVolcanicAshParticles();

    UFUNCTION(BlueprintCallable, Category = "Volcanic Lighting")
    void UpdateLightingForTemperature(float Temperature);

    UFUNCTION(BlueprintPure, Category = "Volcanic Lighting")
    ELight_VolcanicLightState GetCurrentVolcanicState() const { return CurrentVolcanicState; }

    UFUNCTION(BlueprintPure, Category = "Volcanic Lighting")
    float GetCurrentLavaIntensity() const { return CurrentLavaIntensity; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* LavaLightComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* HeatShimmerComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UNiagaraComponent* VolcanicAshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Volcanic Lighting")
    FLight_VolcanicLightConfig VolcanicConfig;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ELight_VolcanicLightState CurrentVolcanicState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentLavaIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentTemperature;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsLavaGlowActive;

private:
    void InitializeVolcanicLighting();
    void UpdateLavaFlicker(float DeltaTime);
    void UpdateHeatDistortion(float DeltaTime);
    void UpdateVolcanicAtmosphere();
    void ApplyTemperatureToLighting(float Temperature);
    FLinearColor CalculateLavaColorForTemperature(float Temperature);

    FTimerHandle FlickerTimerHandle;
    float FlickerTime;
    float BaseFlickerIntensity;
    bool bFlickerDirection;
};