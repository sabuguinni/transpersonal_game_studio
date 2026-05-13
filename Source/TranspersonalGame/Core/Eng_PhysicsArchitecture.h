#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Eng_PhysicsArchitecture.generated.h"

/**
 * ENGINE ARCHITECT CYCLE 008 - PHYSICS ARCHITECTURE
 * 
 * Defines the physics framework for:
 * 1. Dinosaur collision and interaction systems
 * 2. Environmental destruction and deformation
 * 3. Realistic survival mechanics (falling, drowning, crushing)
 * 4. Performance-optimized physics simulation
 * 5. Cross-system physics communication protocols
 * 
 * CRITICAL: All physics implementations must follow this architecture
 */

// Physics Interaction Types for Dinosaur Game
UENUM(BlueprintType)
enum class EEng_PhysicsInteractionType : uint8
{
    None                UMETA(DisplayName = "None"),
    DinosaurCollision   UMETA(DisplayName = "Dinosaur Collision"),
    PlayerInteraction   UMETA(DisplayName = "Player Interaction"),
    Environmental       UMETA(DisplayName = "Environmental"),
    Destructible        UMETA(DisplayName = "Destructible"),
    Fluid               UMETA(DisplayName = "Fluid"),
    Projectile          UMETA(DisplayName = "Projectile"),
    Ragdoll             UMETA(DisplayName = "Ragdoll")
};

// Physics Simulation Levels for Performance
UENUM(BlueprintType)
enum class EEng_PhysicsSimulationLevel : uint8
{
    Disabled        UMETA(DisplayName = "Disabled"),
    Kinematic       UMETA(DisplayName = "Kinematic"),
    Simple          UMETA(DisplayName = "Simple"),
    Complex         UMETA(DisplayName = "Complex"),
    HighFidelity    UMETA(DisplayName = "High Fidelity")
};

// Dinosaur Physics Categories
UENUM(BlueprintType)
enum class EEng_DinosaurPhysicsCategory : uint8
{
    SmallHerbivore      UMETA(DisplayName = "Small Herbivore"),     // <100kg
    MediumHerbivore     UMETA(DisplayName = "Medium Herbivore"),    // 100-1000kg
    LargeHerbivore      UMETA(DisplayName = "Large Herbivore"),     // 1000-10000kg
    MegaHerbivore       UMETA(DisplayName = "Mega Herbivore"),      // >10000kg
    SmallCarnivore      UMETA(DisplayName = "Small Carnivore"),     // <50kg
    MediumCarnivore     UMETA(DisplayName = "Medium Carnivore"),    // 50-500kg
    LargeCarnivore      UMETA(DisplayName = "Large Carnivore"),     // 500-5000kg
    ApexPredator        UMETA(DisplayName = "Apex Predator")        // >5000kg
};

// Physics Material Properties for Prehistoric Environment
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsMaterialProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Friction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Restitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float Density;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    bool bDestructible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    float DestructionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Material")
    FString MaterialType; // "Rock", "Wood", "Bone", "Flesh", "Water", "Mud"

    FEng_PhysicsMaterialProperties()
    {
        Friction = 0.7f;
        Restitution = 0.3f;
        Density = 1.0f;
        bDestructible = false;
        DestructionThreshold = 1000.0f;
        MaterialType = "Generic";
    }
};

// Dinosaur Physics Configuration
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurPhysicsConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    EEng_DinosaurPhysicsCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float Mass; // In kilograms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float MaxRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float JumpForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float StompDamage; // For large dinosaurs

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    float CollisionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    bool bCanDestroyTrees;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    bool bCanDestroyRocks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Physics")
    EEng_PhysicsSimulationLevel SimulationLevel;

    FEng_DinosaurPhysicsConfig()
    {
        Category = EEng_DinosaurPhysicsCategory::MediumHerbivore;
        Mass = 500.0f;
        MaxWalkSpeed = 300.0f;
        MaxRunSpeed = 800.0f;
        JumpForce = 500.0f;
        StompDamage = 0.0f;
        CollisionRadius = 100.0f;
        bCanDestroyTrees = false;
        bCanDestroyRocks = false;
        SimulationLevel = EEng_PhysicsSimulationLevel::Complex;
    }
};

// Environmental Physics Event Data
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsEventData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    EEng_PhysicsInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    FVector ImpactNormal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    AActor* SourceActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Event")
    float Timestamp;

    FEng_PhysicsEventData()
    {
        InteractionType = EEng_PhysicsInteractionType::None;
        ImpactLocation = FVector::ZeroVector;
        ImpactNormal = FVector::UpVector;
        ImpactForce = 0.0f;
        SourceActor = nullptr;
        TargetActor = nullptr;
        EventDescription = "Unknown Event";
        Timestamp = 0.0f;
    }
};

/**
 * PHYSICS ARCHITECTURE SUBSYSTEM
 * 
 * Central authority for all physics-related systems in the dinosaur game.
 * Manages collision detection, environmental destruction, and performance optimization.
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PhysicsArchitecture : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Dinosaur Physics Registration
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool RegisterDinosaurPhysics(AActor* DinosaurActor, const FEng_DinosaurPhysicsConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool UnregisterDinosaurPhysics(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    FEng_DinosaurPhysicsConfig GetDinosaurPhysicsConfig(AActor* DinosaurActor);

    // Environmental Physics
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool RegisterEnvironmentalObject(AActor* EnvironmentActor, const FEng_PhysicsMaterialProperties& Properties);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void ProcessEnvironmentalDestruction(const FEng_PhysicsEventData& EventData);

    // Collision Management
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void HandleDinosaurCollision(AActor* Dinosaur1, AActor* Dinosaur2, const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void HandlePlayerDinosaurCollision(AActor* Player, AActor* Dinosaur, const FHitResult& HitResult);

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void OptimizePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void SetPhysicsLODLevel(AActor* Actor, EEng_PhysicsSimulationLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    int32 GetActivePhysicsBodies();

    // Event System
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void BroadcastPhysicsEvent(const FEng_PhysicsEventData& EventData);

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    TArray<FEng_PhysicsEventData> GetRecentPhysicsEvents(float TimeWindow);

    // Validation and Debugging
    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    bool ValidatePhysicsSetup();

    UFUNCTION(BlueprintCallable, Category = "Physics Architecture")
    void LogPhysicsStatistics();

protected:
    // Registered dinosaur physics configurations
    UPROPERTY()
    TMap<AActor*, FEng_DinosaurPhysicsConfig> DinosaurPhysicsRegistry;

    // Environmental object properties
    UPROPERTY()
    TMap<AActor*, FEng_PhysicsMaterialProperties> EnvironmentalRegistry;

    // Recent physics events for debugging and analysis
    UPROPERTY()
    TArray<FEng_PhysicsEventData> RecentEvents;

    // Performance settings
    UPROPERTY(EditAnywhere, Category = "Physics Architecture")
    int32 MaxPhysicsBodies;

    UPROPERTY(EditAnywhere, Category = "Physics Architecture")
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, Category = "Physics Architecture")
    bool bEnableEnvironmentalDestruction;

    UPROPERTY(EditAnywhere, Category = "Physics Architecture")
    float EventHistoryDuration;

private:
    // Internal optimization methods
    void UpdatePhysicsLOD();
    void CleanupOldEvents();
    void ValidatePhysicsIntegrity();

    // Performance monitoring
    FTimerHandle OptimizationTimer;
    FTimerHandle ValidationTimer;
};