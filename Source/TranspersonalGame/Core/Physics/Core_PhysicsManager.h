#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/Engine.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Static      UMETA(DisplayName = "Static"),
    Kinematic   UMETA(DisplayName = "Kinematic"),
    Simulated   UMETA(DisplayName = "Simulated"),
    Ragdoll     UMETA(DisplayName = "Ragdoll")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Simulated;

    FCore_PhysicsSettings()
    {
        Mass = 100.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.01f;
        bEnableGravity = true;
        PhysicsMode = ECore_PhysicsMode::Simulated;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsManager : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    FCore_PhysicsSettings DefaultSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    float GlobalGravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    bool bEnablePhysicsSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Manager")
    int32 MaxSimulatedBodies = 1000;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Manager")
    int32 CurrentSimulatedBodies = 0;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void ApplyPhysicsSettings(UPrimitiveComponent* Component, const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void EnableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void DisableRagdoll(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void SetGlobalGravityScale(float NewScale);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void AddForceToActor(AActor* Actor, FVector Force, bool bAccelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void AddImpulseToActor(AActor* Actor, FVector Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    bool IsActorSimulatingPhysics(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    int32 GetSimulatedBodiesCount() const { return CurrentSimulatedBodies; }

    UFUNCTION(BlueprintCallable, Category = "Physics Manager")
    void SetMaxSimulatedBodies(int32 NewMax) { MaxSimulatedBodies = NewMax; }

private:
    void UpdateSimulatedBodiesCount();
    void EnforceBodyLimits();
    
    UPROPERTY()
    TArray<TWeakObjectPtr<UPrimitiveComponent>> TrackedComponents;
};