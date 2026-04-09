#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Chaos/ChaosEngineInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "FluidPhysicsSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFluidPhysics, Log, All);

/**
 * Fluid Physics System for water, mud, and other fluid interactions
 * Handles realistic fluid simulation for prehistoric environment
 * Supports character wading, swimming, and environmental fluid effects
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UFluidPhysicsSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UFluidPhysicsSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Initialize fluid simulation */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void InitializeFluidSimulation();

    /** Create fluid volume at location */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void CreateFluidVolume(const FVector& Location, const FVector& Size, float Density = 1000.0f);

    /** Add fluid interaction for actor */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void AddFluidInteraction(AActor* Actor, float BuoyancyForce = 9800.0f);

    /** Remove fluid interaction */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void RemoveFluidInteraction(AActor* Actor);

    /** Check if location is in fluid */
    UFUNCTION(BlueprintPure, Category = "Fluid Physics")
    bool IsLocationInFluid(const FVector& Location, float& FluidDensity) const;

    /** Get fluid velocity at location */
    UFUNCTION(BlueprintPure, Category = "Fluid Physics")
    FVector GetFluidVelocityAtLocation(const FVector& Location) const;

    /** Apply fluid forces to actor */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void ApplyFluidForces(AActor* Actor, float DeltaTime);

    /** Create splash effect */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void CreateSplashEffect(const FVector& Location, const FVector& Velocity, float Intensity = 1.0f);

    /** Set fluid current direction and strength */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void SetFluidCurrent(const FVector& Direction, float Strength);

    /** Enable/disable fluid simulation */
    UFUNCTION(BlueprintCallable, Category = "Fluid Physics")
    void SetFluidSimulationEnabled(bool bEnabled);

protected:
    /** Fluid volumes in the world */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fluid Physics")
    TArray<FBox> FluidVolumes;

    /** Actors currently interacting with fluid */
    UPROPERTY()
    TMap<TObjectPtr<AActor>, float> FluidInteractingActors;

    /** Fluid simulation enabled */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    bool bFluidSimulationEnabled = true;

    /** Default fluid density (kg/m³) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    float DefaultFluidDensity = 1000.0f;

    /** Fluid viscosity coefficient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    float FluidViscosity = 0.001f;

    /** Current direction and strength */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    FVector FluidCurrentDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    float FluidCurrentStrength = 0.0f;

    /** Splash effect template */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Physics")
    TObjectPtr<class UNiagaraSystem> SplashEffectTemplate;

    /** Performance settings */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxFluidInteractions = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FluidUpdateFrequency = 30.0f;

private:
    void UpdateFluidInteractions(float DeltaTime);
    void CalculateBuoyancyForce(AActor* Actor, const FVector& FluidVelocity, float FluidDensity, float DeltaTime);
    void CalculateDragForce(AActor* Actor, const FVector& FluidVelocity, float DeltaTime);
    bool IsActorInFluidVolume(AActor* Actor, FBox& OutFluidVolume) const;
    
    float LastFluidUpdateTime = 0.0f;
    TArray<TObjectPtr<UNiagaraComponent>> ActiveSplashEffects;
};