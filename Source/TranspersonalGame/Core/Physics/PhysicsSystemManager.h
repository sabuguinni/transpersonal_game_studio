#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core/SharedTypes.h"
#include "PhysicsSystemManager.generated.h"

class UPhysicsAsset;
class USkeletalMeshComponent;
class UPhysicsConstraintComponent;

/**
 * Physics System Manager - Core physics integration and ragdoll system
 * Manages physics materials, collision channels, and ragdoll transitions
 * Integrates with TranspersonalCharacter for survival mechanics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsMaterials();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    UPhysicalMaterial* GetPhysicsMaterialForSurface(ECore_SurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsMaterialProperties(UPhysicalMaterial* Material, float Friction, float Restitution, float Density);

    // Collision Channel Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ConfigureCollisionChannels();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetCollisionResponseForChannel(UPrimitiveComponent* Component, ECore_CollisionChannel Channel, bool bBlock);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh, bool bSimulatePhysics = true);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    void DisableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    void ConfigureRagdollConstraints(USkeletalMeshComponent* SkeletalMesh, float LinearLimit, float AngularLimit);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh);

    // Physics Simulation Control
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetGlobalPhysicsSettings(float GravityZ, float LinearDamping, float AngularDamping);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsSimulation(UPrimitiveComponent* Component, bool bSimulate);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetMassProperties(UPrimitiveComponent* Component, float Mass, FVector CenterOfMass);

    // Physics Validation and Debugging
    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug")
    void ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug")
    void DrawPhysicsDebugInfo(bool bShowConstraints, bool bShowCollision, bool bShowMass);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug", CallInEditor)
    void TestPhysicsIntegration();

protected:
    // Physics Materials
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Materials")
    TMap<ECore_SurfaceType, UPhysicalMaterial*> PhysicsMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Materials")
    UPhysicalMaterial* DefaultPhysicsMaterial;

    // Ragdoll Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollAngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ragdoll")
    float RagdollMassScale;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float WorldGravityZ;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float DefaultLinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Settings")
    float DefaultAngularDamping;

    // Active Ragdolls Tracking
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll")
    TArray<USkeletalMeshComponent*> ActiveRagdolls;

private:
    void CreateDefaultPhysicsMaterials();
    void LoadPhysicsMaterialAssets();
    void SetupCollisionProfiles();
    void InitializeRagdollSettings();

    // Physics material creation helpers
    UPhysicalMaterial* CreatePhysicsMaterial(const FString& Name, float Friction, float Restitution, float Density);
    void ConfigurePhysicsMaterialProperties(UPhysicalMaterial* Material, ECore_SurfaceType SurfaceType);

    bool bPhysicsSystemInitialized;
    bool bRagdollSystemEnabled;
};