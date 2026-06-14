#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic      UMETA(DisplayName = "Realistic"),
    Arcade         UMETA(DisplayName = "Arcade"),
    Cinematic      UMETA(DisplayName = "Cinematic")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        MaxAngularVelocity = 3600.0f;
        bEnableGravity = true;
        bSimulatePhysics = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics Mode Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Mode")
    ECore_PhysicsMode CurrentPhysicsMode = ECore_PhysicsMode::Realistic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings RealisticSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings ArcadeSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings CinematicSettings;

    // Physics Control Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettings(const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FCore_PhysicsSettings GetCurrentPhysicsSettings() const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalGravity(float NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetGlobalGravity() const;

    // Physics Simulation Control
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void EnablePhysicsSimulation(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsPhysicsSimulationEnabled() const;

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsMaterial(class UPhysicalMaterial* NewMaterial);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    class UPhysicalMaterial* GetPhysicsMaterial() const;

    // Force Application
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyForceToActor(AActor* TargetActor, FVector Force, bool bAccelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyImpulseToActor(AActor* TargetActor, FVector Impulse, bool bVelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyTorqueToActor(AActor* TargetActor, FVector Torque, bool bAccelChange = false);

    // Physics Query Functions
    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool IsActorPhysicsEnabled(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetActorVelocity(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FVector GetActorAngularVelocity(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetActorMass(AActor* TargetActor) const;

private:
    // Internal state
    UPROPERTY()
    class UPhysicalMaterial* CurrentPhysicsMaterial;

    UPROPERTY()
    bool bPhysicsEnabled = true;

    // Helper functions
    void InitializePhysicsSettings();
    void UpdateWorldPhysicsSettings();
    class UPrimitiveComponent* GetActorPrimitiveComponent(AActor* Actor) const;
};

#include "Core_PhysicsManager.generated.h"