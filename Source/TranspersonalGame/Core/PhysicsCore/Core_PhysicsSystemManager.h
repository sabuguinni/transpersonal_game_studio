#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "../SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

/**
 * Physics System Manager for Transpersonal Game Studio
 * Manages physics simulation, collision detection, and material properties
 * Integrates with Engine Architect's system management framework
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics system management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void EnablePhysicsOnActor(AActor* Actor, bool bSimulatePhysics = true);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetActorPhysicsMaterial(AActor* Actor, class UPhysicalMaterial* PhysicsMaterial);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ConfigureCollisionForActor(AActor* Actor, ECollisionEnabled::Type CollisionType);

    // Prehistoric physics materials
    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetRockPhysicsMaterial();

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetWoodPhysicsMaterial();

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetGroundPhysicsMaterial();

    UFUNCTION(BlueprintCallable, Category = "Physics Materials")
    class UPhysicalMaterial* GetWaterPhysicsMaterial();

    // Dinosaur-specific physics
    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void ConfigureDinosaurPhysics(AActor* DinosaurActor, float Mass, float LinearDamping);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur Physics")
    void SetDinosaurCollisionProfile(AActor* DinosaurActor, FName CollisionProfile);

    // Environmental physics
    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void EnableDestructiblePhysics(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Environmental Physics")
    void ConfigureVegetationPhysics(AActor* VegetationActor);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetActivePhysicsBodies();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsSimulationTime();

    // Integration with Engine Architect systems
    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithSystemManager();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    bool IsPhysicsSystemReady();

protected:
    // Physics materials cache
    UPROPERTY()
    class UPhysicalMaterial* RockPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* WoodPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* GroundPhysicsMaterial;

    UPROPERTY()
    class UPhysicalMaterial* WaterPhysicsMaterial;

    // System state
    UPROPERTY(BlueprintReadOnly, Category = "System State")
    bool bIsInitialized;

    UPROPERTY(BlueprintReadOnly, Category = "System State")
    int32 ManagedActorsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float LastFramePhysicsTime;

private:
    // Internal physics material creation
    void CreatePhysicsMaterials();
    void SetupDefaultCollisionProfiles();
    
    // Performance tracking
    void UpdatePhysicsMetrics();
    
    // Actor management
    TArray<TWeakObjectPtr<AActor>> ManagedActors;
    
    // System integration
    bool bRegisteredWithSystemManager;
};

/**
 * Physics Configuration Data Structure
 * Stores physics settings for different object types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    FName CollisionProfileName = "BlockAll";

    FCore_PhysicsConfig()
    {
        Mass = 100.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        CollisionProfileName = "BlockAll";
    }
};

/**
 * Prehistoric Physics Material Types
 * Defines physics properties for different materials in the prehistoric world
 */
UENUM(BlueprintType)
enum class ECore_PhysicsMaterialType : uint8
{
    Rock        UMETA(DisplayName = "Rock"),
    Wood        UMETA(DisplayName = "Wood"),
    Ground      UMETA(DisplayName = "Ground"),
    Water       UMETA(DisplayName = "Water"),
    Bone        UMETA(DisplayName = "Bone"),
    Flesh       UMETA(DisplayName = "Flesh"),
    Vegetation  UMETA(DisplayName = "Vegetation"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Ice         UMETA(DisplayName = "Ice")
};