#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Core/Engine/Eng_SystemArchitect.h"
#include "Core_TerrainPhysics.generated.h"

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water"),
    Lava        UMETA(DisplayName = "Lava"),
    Ice         UMETA(DisplayName = "Ice")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float Density = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float MovementSpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float StabilityFactor = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bCanSink = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float SinkRate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float TemperatureEffect = 0.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainInteraction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    AActor* InteractingActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    FVector ContactPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    float InteractionStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    float Duration = 0.0f;
};

/**
 * Core Terrain Physics System
 * Manages realistic terrain physics interactions for the prehistoric survival game
 * Handles terrain material properties, character movement on different surfaces,
 * environmental effects, and realistic physics simulation
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_TerrainPhysics : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_TerrainPhysics();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === TERRAIN PHYSICS PROPERTIES ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    TMap<ECore_TerrainType, FCore_TerrainPhysicsProperties> TerrainProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float TerrainDetectionRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float PhysicsUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    bool bEnableRealtimePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Physics")
    float GravityMultiplier = 1.0f;

    // === TERRAIN INTERACTION SYSTEM ===

    UPROPERTY(BlueprintReadOnly, Category = "Terrain Interaction")
    TArray<FCore_TerrainInteraction> ActiveInteractions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    int32 MaxSimultaneousInteractions = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Interaction")
    float InteractionCleanupInterval = 5.0f;

    // === PERFORMANCE OPTIMIZATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance1 = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance2 = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance3 = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODOptimization = true;

    // === SYSTEM REGISTRATION ===

    UPROPERTY(BlueprintReadOnly, Category = "System Registration")
    class UEng_SystemArchitect* SystemArchitect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Registration")
    bool bRegisteredWithArchitect = false;

public:
    // === TERRAIN PHYSICS FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    ECore_TerrainType DetectTerrainType(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    FCore_TerrainPhysicsProperties GetTerrainProperties(ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ApplyTerrainPhysics(AActor* Actor, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    float CalculateMovementModifier(ECore_TerrainType TerrainType, float BaseSpeed);

    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void UpdateTerrainInteraction(AActor* Actor, const FVector& ContactPoint);

    // === TERRAIN MATERIAL FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Terrain Materials")
    void SetTerrainMaterialProperties(ECore_TerrainType TerrainType, float Friction, float Restitution);

    UFUNCTION(BlueprintCallable, Category = "Terrain Materials")
    void ApplyEnvironmentalEffects(const FVector& Location, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Terrain Materials")
    bool IsTerrainStable(const FVector& Location, float ActorMass = 100.0f);

    // === PHYSICS SIMULATION FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void SimulateTerrainDeformation(const FVector& ImpactPoint, float Force, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void UpdateGravityEffects(AActor* Actor, ECore_TerrainType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Physics Simulation")
    void ProcessTerrainCollisions(float DeltaTime);

    // === PERFORMANCE FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupInactiveInteractions();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveInteractionCount() const;

    // === SYSTEM INTEGRATION FUNCTIONS ===

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void RegisterWithSystemArchitect();

    UFUNCTION(BlueprintCallable, Category = "System Integration")
    void ValidateSystemIntegrity();

    UFUNCTION(BlueprintCallable, Category = "System Integration", CallInEditor = true)
    void RunTerrainPhysicsTests();

    // === BLUEPRINT EVENTS ===

    UFUNCTION(BlueprintImplementableEvent, Category = "Terrain Events")
    void OnTerrainTypeChanged(ECore_TerrainType NewTerrainType, const FVector& Location);

    UFUNCTION(BlueprintImplementableEvent, Category = "Terrain Events")
    void OnTerrainDeformation(const FVector& Location, float DeformationAmount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Terrain Events")
    void OnUnstableTerrainDetected(const FVector& Location, AActor* AffectedActor);

private:
    // Internal tracking
    float LastCleanupTime = 0.0f;
    int32 CurrentLODLevel = 0;
    TArray<AActor*> TrackedActors;
    
    // Performance monitoring
    float AverageFrameTime = 0.0f;
    int32 FrameCounter = 0;
    
    // Helper functions
    void InitializeTerrainProperties();
    void UpdatePerformanceMetrics(float DeltaTime);
    ECore_TerrainType SampleTerrainAtLocation(const FVector& Location);
    void ApplyTerrainSpecificPhysics(AActor* Actor, ECore_TerrainType TerrainType);
};