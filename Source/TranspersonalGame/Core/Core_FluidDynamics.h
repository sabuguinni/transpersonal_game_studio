#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Core_FluidDynamics.generated.h"

/**
 * Core Fluid Dynamics System
 * Handles water simulation, river flow, rain effects, and fluid interactions
 * Optimized for prehistoric environments with realistic water physics
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_FluidProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float Viscosity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float Density = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float FlowRate = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    bool bIsContaminated = false;

    FCore_FluidProperties()
    {
        Viscosity = 1.0f;
        Density = 1000.0f;
        FlowRate = 100.0f;
        Temperature = 20.0f;
        bIsContaminated = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WaterCurrentData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Current")
    FVector FlowDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Current")
    float CurrentStrength = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Current")
    float WaveHeight = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Current")
    float WaveFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Current")
    bool bHasUndercurrent = false;

    FCore_WaterCurrentData()
    {
        FlowDirection = FVector::ZeroVector;
        CurrentStrength = 50.0f;
        WaveHeight = 10.0f;
        WaveFrequency = 1.0f;
        bHasUndercurrent = false;
    }
};

UENUM(BlueprintType)
enum class ECore_FluidType : uint8
{
    Water           UMETA(DisplayName = "Water"),
    Mud             UMETA(DisplayName = "Mud"),
    Quicksand       UMETA(DisplayName = "Quicksand"),
    BloodPool       UMETA(DisplayName = "Blood Pool"),
    TarPit          UMETA(DisplayName = "Tar Pit"),
    HotSpring       UMETA(DisplayName = "Hot Spring")
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_FluidDynamics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_FluidDynamics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Fluid Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    ECore_FluidType FluidType = ECore_FluidType::Water;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    FCore_FluidProperties FluidProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    FCore_WaterCurrentData CurrentData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    float FluidLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    bool bSimulateFlow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    bool bAffectsCharacterMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics", meta = (AllowPrivateAccess = "true"))
    float DragCoefficient = 0.5f;

    // Fluid Simulation Methods
    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void InitializeFluidSystem();

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void UpdateFluidFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SimulateWaterCurrent(const FVector& InputForce);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void ApplyFluidForceToActor(AActor* TargetActor, float ForceMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    bool IsActorInFluid(AActor* TestActor) const;

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    float CalculateFluidDrag(const FVector& Velocity) const;

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SetFluidProperties(const FCore_FluidProperties& NewProperties);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    FCore_FluidProperties GetFluidProperties() const;

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void CreateRippleEffect(const FVector& ImpactLocation, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SimulateRainEffect(float RainIntensity, const FVector& RainDirection);

    // River and Stream Simulation
    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SetupRiverFlow(const TArray<FVector>& FlowPath, float FlowSpeed);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void UpdateRiverCurrent(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    FVector GetCurrentVelocityAtLocation(const FVector& Location) const;

    // Prehistoric Environment Specific
    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SimulateTarPitEffect(AActor* TrappedActor);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SimulateQuicksandSinking(AActor* SinkingActor, float SinkRate);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void CreateHotSpringBubbles(const FVector& SpringLocation);

    // Performance and Optimization
    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void OptimizeFluidSimulation(int32 MaxParticles = 1000);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SetFluidLOD(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    bool ValidateFluidSystem() const;

private:
    // Internal simulation data
    TArray<FVector> FlowPath;
    float CurrentFlowSpeed = 100.0f;
    bool bIsFluidSystemActive = false;
    float LastUpdateTime = 0.0f;
    int32 CurrentLODLevel = 0;

    // Internal helper methods
    void UpdateInternalFluidState(float DeltaTime);
    FVector CalculateFluidForce(const FVector& ActorVelocity) const;
    void ProcessFluidInteractions();
    bool IsLocationInFluidBounds(const FVector& Location) const;
};

/**
 * Fluid Dynamics Actor - Standalone fluid simulation actor
 * Can be placed in levels to create water bodies, rivers, tar pits, etc.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_FluidDynamicsActor : public AActor
{
    GENERATED_BODY()

public:
    ACore_FluidDynamicsActor();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UStaticMeshComponent* FluidMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UCore_FluidDynamics* FluidDynamicsComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    class UBoxComponent* FluidBounds;

    // Fluid Actor Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Setup")
    bool bAutoStartSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Setup")
    float FluidDepth = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Setup")
    FVector FluidDimensions = FVector(1000.0f, 1000.0f, 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Fluid Actor")
    void SetupFluidBounds(const FVector& Dimensions);

    UFUNCTION(BlueprintCallable, Category = "Fluid Actor")
    void StartFluidSimulation();

    UFUNCTION(BlueprintCallable, Category = "Fluid Actor")
    void StopFluidSimulation();

    UFUNCTION(BlueprintCallable, Category = "Fluid Actor")
    bool IsFluidSimulationActive() const;
};