#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Particles/ParticleSystemComponent.h"
#include "Light_TorchLightingSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_TorchState : uint8
{
    Unlit,
    Lighting,
    Burning,
    Dying,
    Extinguished
};

USTRUCT(BlueprintType)
struct FLight_TorchFlickerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Flicker")
    float BaseIntensity = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Flicker")
    float FlickerAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Flicker")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Flicker")
    FLinearColor BaseColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch Flicker")
    float AttenuationRadius = 800.0f;

    FLight_TorchFlickerData()
    {
        BaseIntensity = 2000.0f;
        FlickerAmount = 0.3f;
        FlickerSpeed = 2.0f;
        BaseColor = FLinearColor(1.0f, 0.6f, 0.3f, 1.0f);
        AttenuationRadius = 800.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_TorchLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_TorchLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* TorchMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* TorchLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* FlameParticles;

    // Torch State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch State")
    ELight_TorchState CurrentState = ELight_TorchState::Unlit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch State")
    float FuelRemaining = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch State")
    float MaxFuel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Torch State")
    float FuelConsumptionRate = 1.0f;

    // Flicker Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLight_TorchFlickerData FlickerData;

    // Lighting Functions
    UFUNCTION(BlueprintCallable, Category = "Torch")
    void LightTorch();

    UFUNCTION(BlueprintCallable, Category = "Torch")
    void ExtinguishTorch();

    UFUNCTION(BlueprintCallable, Category = "Torch")
    void AddFuel(float Amount);

    UFUNCTION(BlueprintCallable, Category = "Torch")
    bool CanBeLit() const;

    UFUNCTION(BlueprintPure, Category = "Torch")
    float GetFuelPercentage() const;

protected:
    // Internal Functions
    void UpdateFlicker(float DeltaTime);
    void UpdateFuelConsumption(float DeltaTime);
    void UpdateTorchState();
    void SetTorchState(ELight_TorchState NewState);

    // Timer Handles
    FTimerHandle FlickerTimerHandle;
    FTimerHandle FuelTimerHandle;

    // Flicker Variables
    float FlickerTime = 0.0f;
    float CurrentIntensityMultiplier = 1.0f;

    // Weather Interaction
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectedByRain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
    bool bAffectedByWind = true;

public:
    // Weather Response
    UFUNCTION(BlueprintCallable, Category = "Weather")
    void OnRainStart();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void OnRainEnd();

    UFUNCTION(BlueprintCallable, Category = "Weather")
    void OnWindChange(float WindStrength);
};