#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Arcade          UMETA(DisplayName = "Arcade Physics"),
    Cinematic       UMETA(DisplayName = "Cinematic Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Realistic;
};

UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float WorldGravityZ = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxPhysicsSubsteps = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float MaxSubstepDeltaTime = 0.016667f;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetWorldGravity(float NewGravityZ);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettingsToActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsForActor(AActor* TargetActor, bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector CalculateImpactForce(const FVector& Velocity, float Mass, float ImpactDuration = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SimulateExplosion(const FVector& ExplosionLocation, float ExplosionRadius, float ExplosionForce);

private:
    UPROPERTY()
    TArray<AActor*> ManagedActors;

    void UpdatePhysicsSettings();
    void ValidatePhysicsComponents();
};