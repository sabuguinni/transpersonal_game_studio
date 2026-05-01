#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Physics material types for different creature categories
 * Used to define realistic physics properties for dinosaurs and environment
 */
UENUM(BlueprintType)
enum class ECore_PhysicsMaterialType : uint8
{
    TRex           UMETA(DisplayName = "T-Rex Physics"),
    Raptor         UMETA(DisplayName = "Raptor Physics"),
    Brachiosaurus  UMETA(DisplayName = "Brachiosaurus Physics"),
    Rock           UMETA(DisplayName = "Rock Physics"),
    Wood           UMETA(DisplayName = "Wood Physics"),
    Terrain        UMETA(DisplayName = "Terrain Physics")
};

/**
 * Ragdoll state for character physics simulation
 * Controls when characters transition to physics-based animation
 */
UENUM(BlueprintType)
enum class ECore_RagdollState : uint8
{
    Disabled       UMETA(DisplayName = "Ragdoll Disabled"),
    Enabled        UMETA(DisplayName = "Ragdoll Enabled"),
    Transitioning  UMETA(DisplayName = "Transitioning to Ragdoll")
};

/**
 * Physics properties for different creature types
 * Defines mass, friction, and collision behavior
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsProperties
{
    GENERATED_BODY()

    /** Mass in kilograms */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MassKg = 100.0f;

    /** Surface friction coefficient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction = 0.7f;

    /** Bounce/restitution coefficient */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution = 0.3f;

    /** Material density */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Density = 1.0f;

    /** Whether this object can be pushed by other physics objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bCanBePushed = true;

    FCore_PhysicsProperties()
    {
        MassKg = 100.0f;
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        bCanBePushed = true;
    }
};

/**
 * Core Physics System Manager
 * Manages all physics simulation, collision detection, and ragdoll systems
 * Handles realistic physics for dinosaurs, environment, and character death
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    /** Initialize physics materials for all creature types */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsMaterials();

    /** Apply physics properties to a specific actor */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool ApplyPhysicsProperties(AActor* Actor, ECore_PhysicsMaterialType MaterialType);

    /** Enable ragdoll physics on a character */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool EnableRagdoll(AActor* Character);

    /** Disable ragdoll physics on a character */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool DisableRagdoll(AActor* Character);

    /** Get physics properties for a material type */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    FCore_PhysicsProperties GetPhysicsProperties(ECore_PhysicsMaterialType MaterialType) const;

    /** Set up collision channels for the game */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetupCollisionChannels();

    /** Test physics simulation on all objects */
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TestPhysicsSimulation();

    /** Get the current ragdoll state of a character */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Physics System")
    ECore_RagdollState GetRagdollState(AActor* Character) const;

protected:
    /** Physics materials for different creature types */
    UPROPERTY(BlueprintReadOnly, Category = "Physics Materials")
    TMap<ECore_PhysicsMaterialType, UPhysicalMaterial*> PhysicsMaterials;

    /** Physics properties for each material type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Properties")
    TMap<ECore_PhysicsMaterialType, FCore_PhysicsProperties> MaterialProperties;

    /** Actors currently in ragdoll state */
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll System")
    TMap<AActor*, ECore_RagdollState> RagdollStates;

private:
    /** Create a physics material with specific properties */
    UPhysicalMaterial* CreatePhysicsMaterial(const FString& MaterialName, const FCore_PhysicsProperties& Properties);

    /** Apply collision settings to a static mesh component */
    void ApplyCollisionSettings(UStaticMeshComponent* MeshComp, ECore_PhysicsMaterialType MaterialType);

    /** Apply collision settings to a skeletal mesh component */
    void ApplyCollisionSettings(USkeletalMeshComponent* MeshComp, ECore_PhysicsMaterialType MaterialType);

    /** Initialize default physics properties */
    void InitializeDefaultProperties();
};