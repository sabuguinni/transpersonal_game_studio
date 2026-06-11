#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Core Physics System Manager
 * Manages global physics settings, collision profiles, and physics materials
 * Handles ragdoll activation, destructible systems, and physics optimization
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS CONFIGURATION ===
    
    /** Global gravity multiplier for the world */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|World", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float GravityMultiplier = 1.0f;
    
    /** Enable/disable physics simulation globally */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|World")
    bool bPhysicsEnabled = true;
    
    /** Maximum number of physics substeps per frame */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Performance", meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaxPhysicsSubsteps = 4;
    
    /** Physics simulation timestep */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Performance", meta = (ClampMin = "0.008", ClampMax = "0.033"))
    float PhysicsTimestep = 0.016f;

    // === COLLISION SYSTEM ===
    
    /** Default collision profile for dynamic objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Collision")
    FName DefaultDynamicProfile = "BlockAll";
    
    /** Default collision profile for static objects */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Collision")
    FName DefaultStaticProfile = "BlockAllDynamic";
    
    /** Enable complex collision for static meshes */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Collision")
    bool bUseComplexCollisionForStatic = true;

    // === RAGDOLL SYSTEM ===
    
    /** Enable ragdoll physics for characters */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll")
    bool bRagdollEnabled = true;
    
    /** Time before ragdoll automatically disables (0 = never) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll", meta = (ClampMin = "0.0", ClampMax = "60.0"))
    float RagdollTimeout = 10.0f;
    
    /** Ragdoll linear damping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float RagdollLinearDamping = 0.1f;
    
    /** Ragdoll angular damping */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll", meta = (ClampMin = "0.0", ClampMax = "10.0"))
    float RagdollAngularDamping = 0.1f;

    // === DESTRUCTIBLE SYSTEM ===
    
    /** Enable destructible mesh support */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Destruction")
    bool bDestructibleEnabled = true;
    
    /** Maximum number of destructible chunks per object */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Destruction", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxDestructibleChunks = 20;
    
    /** Time before destructible debris is cleaned up */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Destruction", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float DebrisLifetime = 30.0f;

    // === PHYSICS MATERIALS ===
    
    /** Default physics material for organic surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Materials")
    class UPhysicalMaterial* OrganicMaterial;
    
    /** Default physics material for stone surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Materials")
    class UPhysicalMaterial* StoneMaterial;
    
    /** Default physics material for metal surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Materials")
    class UPhysicalMaterial* MetalMaterial;
    
    /** Default physics material for water surfaces */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Materials")
    class UPhysicalMaterial* WaterMaterial;

    // === PUBLIC INTERFACE ===
    
    /** Initialize physics system with world settings */
    UFUNCTION(BlueprintCallable, Category = "Physics|System")
    void InitializePhysicsSystem();
    
    /** Apply global physics settings to the world */
    UFUNCTION(BlueprintCallable, Category = "Physics|System")
    void ApplyPhysicsSettings();
    
    /** Enable ragdoll physics on a skeletal mesh component */
    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    bool EnableRagdoll(class USkeletalMeshComponent* SkeletalMesh);
    
    /** Disable ragdoll physics on a skeletal mesh component */
    UFUNCTION(BlueprintCallable, Category = "Physics|Ragdoll")
    bool DisableRagdoll(class USkeletalMeshComponent* SkeletalMesh);
    
    /** Create destructible version of a static mesh */
    UFUNCTION(BlueprintCallable, Category = "Physics|Destruction")
    class AActor* CreateDestructibleFromMesh(class UStaticMesh* SourceMesh, FVector Location, FRotator Rotation);
    
    /** Apply physics material to a primitive component */
    UFUNCTION(BlueprintCallable, Category = "Physics|Materials")
    void ApplyPhysicsMaterial(class UPrimitiveComponent* Component, class UPhysicalMaterial* Material);
    
    /** Get physics material by surface type */
    UFUNCTION(BlueprintCallable, Category = "Physics|Materials")
    class UPhysicalMaterial* GetPhysicsMaterialBySurface(const FString& SurfaceType);

    // === PERFORMANCE MONITORING ===
    
    /** Get current physics performance stats */
    UFUNCTION(BlueprintCallable, Category = "Physics|Performance")
    void GetPhysicsStats(int32& ActiveBodies, int32& Constraints, float& SimulationTime);
    
    /** Enable/disable physics performance profiling */
    UFUNCTION(BlueprintCallable, Category = "Physics|Performance")
    void SetPhysicsProfilingEnabled(bool bEnabled);

protected:
    // === INTERNAL STATE ===
    
    /** Cached world reference */
    UPROPERTY()
    class UWorld* CachedWorld;
    
    /** Active ragdoll components being tracked */
    UPROPERTY()
    TArray<class USkeletalMeshComponent*> ActiveRagdolls;
    
    /** Ragdoll activation timestamps */
    UPROPERTY()
    TArray<float> RagdollTimestamps;
    
    /** Active destructible debris actors */
    UPROPERTY()
    TArray<class AActor*> ActiveDebris;
    
    /** Debris spawn timestamps */
    UPROPERTY()
    TArray<float> DebrisTimestamps;

    // === INTERNAL METHODS ===
    
    /** Update ragdoll timeout system */
    void UpdateRagdollSystem(float DeltaTime);
    
    /** Update destructible debris cleanup */
    void UpdateDebrisSystem(float DeltaTime);
    
    /** Load default physics materials */
    void LoadDefaultPhysicsMaterials();
    
    /** Configure world physics settings */
    void ConfigureWorldPhysics();
    
    /** Setup collision profiles */
    void SetupCollisionProfiles();
};