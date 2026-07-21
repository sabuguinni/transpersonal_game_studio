#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Core_FluidDynamics.generated.h"

USTRUCT(BlueprintType)
struct FCore_FluidProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float Density = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float Viscosity = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float BuoyancyForce = 9800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float DragCoefficient = 0.47f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    FVector FlowDirection = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Properties")
    float FlowSpeed = 0.0f;

    FCore_FluidProperties()
    {
        Density = 1000.0f;
        Viscosity = 0.001f;
        BuoyancyForce = 9800.0f;
        DragCoefficient = 0.47f;
        FlowDirection = FVector::ZeroVector;
        FlowSpeed = 0.0f;
    }
};

UENUM(BlueprintType)
enum class ECore_FluidType : uint8
{
    Water       UMETA(DisplayName = "Water"),
    Mud         UMETA(DisplayName = "Mud"),
    Quicksand   UMETA(DisplayName = "Quicksand"),
    Lava        UMETA(DisplayName = "Lava"),
    Tar         UMETA(DisplayName = "Tar")
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_FluidDynamics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_FluidDynamics();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Fluid type and properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics")
    ECore_FluidType FluidType = ECore_FluidType::Water;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics")
    FCore_FluidProperties FluidProperties;

    // Fluid volume bounds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics")
    FVector FluidBoundsMin = FVector(-1000, -1000, -100);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics")
    FVector FluidBoundsMax = FVector(1000, 1000, 100);

    // Water level for buoyancy calculations
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fluid Dynamics")
    float WaterLevel = 0.0f;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void InitializeFluidSystem();

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    bool IsActorInFluid(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    float GetFluidDepthAtLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void ApplyBuoyancyForce(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void ApplyDragForce(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void ApplyFlowForce(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    FVector CalculateBuoyancyForce(AActor* Actor, float SubmergedVolume);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    FVector CalculateDragForce(AActor* Actor, const FVector& Velocity);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void SetFluidType(ECore_FluidType NewFluidType);

    UFUNCTION(BlueprintCallable, Category = "Fluid Dynamics")
    void UpdateFluidProperties();

private:
    // Internal fluid calculations
    float CalculateSubmergedVolume(AActor* Actor);
    void ApplyFluidEffects(AActor* Actor, float DeltaTime);
    void UpdateFluidFlow(float DeltaTime);
    
    // Cached values
    TArray<AActor*> ActorsInFluid;
    float LastUpdateTime;
};