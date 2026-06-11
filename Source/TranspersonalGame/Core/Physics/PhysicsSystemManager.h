#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsLayer : uint8
{
    Default         UMETA(DisplayName = "Default"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Environment     UMETA(DisplayName = "Environment"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Ragdoll         UMETA(DisplayName = "Ragdoll")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMaterial
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    FString MaterialName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float Density = 1.0f;

    FCore_PhysicsMaterial()
    {
        MaterialName = TEXT("Default");
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bEnableRagdoll = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollBlendTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoDisableRagdoll = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float AutoDisableTime = 5.0f;
};

/**
 * Core Physics System Manager
 * Manages physics simulation, collision layers, ragdoll systems, and physics materials
 * Handles realistic physics for prehistoric survival gameplay
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
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics Layer Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetupCollisionLayers();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetActorPhysicsLayer(AActor* Actor, ECore_PhysicsLayer Layer);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    ECore_PhysicsLayer GetActorPhysicsLayer(AActor* Actor) const;

    // Physics Materials
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void RegisterPhysicsMaterial(const FString& MaterialName, const FCore_PhysicsMaterial& MaterialData);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    FCore_PhysicsMaterial GetPhysicsMaterial(const FString& MaterialName) const;

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsMaterial(UStaticMeshComponent* MeshComponent, const FString& MaterialName);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh, const FCore_RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    void DisableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Ragdoll")
    bool IsRagdollActive(USkeletalMeshComponent* SkeletalMesh) const;

    // Physics Simulation Control
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsSimulation(AActor* Actor, bool bSimulate);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyForceToActor(AActor* Actor, FVector Force, bool bAccelChange = false);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyImpulseToActor(AActor* Actor, FVector Impulse, bool bVelChange = false);

    // Collision Detection
    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool LineTrace(FVector Start, FVector End, FHitResult& OutHit, ECore_PhysicsLayer LayerMask = ECore_PhysicsLayer::Default);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    bool SphereTrace(FVector Start, FVector End, float Radius, FHitResult& OutHit, ECore_PhysicsLayer LayerMask = ECore_PhysicsLayer::Default);

    // Physics Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void OptimizePhysicsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsLOD(AActor* Actor, int32 LODLevel);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Physics", CallInEditor = true)
    void DebugDrawPhysicsInfo();

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void LogPhysicsStats();

protected:
    // Physics Materials Registry
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    TMap<FString, FCore_PhysicsMaterial> PhysicsMaterials;

    // Active Ragdoll Actors
    UPROPERTY()
    TMap<USkeletalMeshComponent*, FCore_RagdollSettings> ActiveRagdolls;

    // Physics Layer Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics")
    TMap<ECore_PhysicsLayer, int32> PhysicsLayerMapping;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxSimulatingActors = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    bool bUseAdaptivePhysics = true;

private:
    void InitializeDefaultPhysicsMaterials();
    void SetupDefaultCollisionChannels();
    void UpdateRagdollSystems(float DeltaTime);
    
    FTimerHandle RagdollUpdateTimer;
    
    // Performance tracking
    int32 CurrentSimulatingActors = 0;
    float LastPhysicsUpdateTime = 0.0f;
};