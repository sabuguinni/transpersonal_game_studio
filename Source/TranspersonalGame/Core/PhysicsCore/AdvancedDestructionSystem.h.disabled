// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "Field/FieldSystemComponent.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "Subsystems/WorldSubsystem.h"
#include "AdvancedDestructionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAdvancedDestruction, Log, All);

/**
 * Destruction trigger types
 */
UENUM(BlueprintType)
enum class EDestructionTriggerType : uint8
{
    Impact = 0,         // Triggered by physical impact
    Damage = 1,         // Triggered by damage threshold
    Timer = 2,          // Triggered by time delay
    Manual = 3,         // Triggered manually
    Proximity = 4       // Triggered by proximity to other objects
};

/**
 * Destruction patterns for different materials
 */
UENUM(BlueprintType)
enum class EDestructionPattern : uint8
{
    Random = 0,         // Random fracture pattern
    Radial = 1,         // Radial fracture from impact point
    Planar = 2,         // Planar cuts
    Voronoi = 3,        // Voronoi diagram based
    Clustered = 4       // Clustered fracture
};

/**
 * Material types for destruction behavior
 */
UENUM(BlueprintType)
enum class EDestructionMaterialType : uint8
{
    Stone = 0,          // Stone/rock materials
    Wood = 1,           // Wooden materials
    Metal = 2,          // Metal materials
    Glass = 3,          // Glass/brittle materials
    Concrete = 4,       // Concrete materials
    Organic = 5         // Organic materials (trees, etc.)
};

/**
 * Destruction event data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDestructionEventData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    AActor* DestroyedActor = nullptr;

    UPROPERTY(BlueprintReadOnly)
    EDestructionTriggerType TriggerType = EDestructionTriggerType::Impact;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    FVector ImpactDirection = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly)
    float ImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly)
    EDestructionMaterialType MaterialType = EDestructionMaterialType::Stone;

    UPROPERTY(BlueprintReadOnly)
    int32 FragmentCount = 0;

    UPROPERTY(BlueprintReadOnly)
    float TimeStamp = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionEvent, const FDestructionEventData&, EventData);

/**
 * Destruction configuration for different materials
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDestructionMaterialConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDestructionMaterialType MaterialType = EDestructionMaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDestructionPattern FracturePattern = EDestructionPattern::Random;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinFragments = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FragmentLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableParticleEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnableSoundEffects = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ImpulseStrength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FragmentMassScale = 1.0f;
};

/**
 * Advanced Destruction System - Manages Chaos Destruction with enhanced features
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAdvancedDestructionSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UAdvancedDestructionSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core destruction management
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void InitializeDestructionSystem();

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ShutdownDestructionSystem();

    // Destruction configuration
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void RegisterDestructibleActor(AActor* Actor, const FDestructionMaterialConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void UnregisterDestructibleActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void SetMaterialConfig(EDestructionMaterialType MaterialType, const FDestructionMaterialConfig& Config);

    UFUNCTION(BlueprintPure, Category = "Advanced Destruction")
    FDestructionMaterialConfig GetMaterialConfig(EDestructionMaterialType MaterialType) const;

    // Destruction triggers
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void TriggerDestruction(AActor* Actor, EDestructionTriggerType TriggerType, const FVector& ImpactLocation = FVector::ZeroVector, const FVector& ImpactDirection = FVector::ZeroVector, float ImpactForce = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void TriggerRadialDestruction(const FVector& Location, float Radius, float Force, EDestructionMaterialType MaterialFilter = EDestructionMaterialType::Stone);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void TriggerChainDestruction(AActor* InitialActor, float ChainRadius = 500.0f, float ChainForce = 300.0f);

    // Geometry Collection management
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    AGeometryCollectionActor* CreateGeometryCollectionFromActor(AActor* SourceActor, const FDestructionMaterialConfig& Config);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void FractureGeometryCollection(AGeometryCollectionActor* GeometryActor, const FVector& ImpactLocation, EDestructionPattern Pattern);

    // Field system integration
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ApplyDestructionField(const FVector& Location, float Radius, float Strength, EDestructionTriggerType TriggerType);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ApplyRadialForceField(const FVector& Location, float Radius, float Force);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ApplyDirectionalForceField(const FVector& Location, const FVector& Direction, float Force, float Radius);

    // Performance management
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void SetDestructionQuality(int32 QualityLevel); // 0=Low, 1=Medium, 2=High, 3=Ultra

    UFUNCTION(BlueprintPure, Category = "Advanced Destruction")
    int32 GetActiveFragmentCount() const { return ActiveFragmentCount; }

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void CleanupOldFragments(float MaxAge = 30.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void SetMaxActiveFragments(int32 MaxFragments) { MaxActiveFragments = MaxFragments; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Advanced Destruction")
    FOnDestructionEvent OnDestructionEvent;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void EnableDestructionDebugVisualization(bool bEnable = true);

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    void ValidateDestructionConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Advanced Destruction")
    bool RunDestructionTest(EDestructionMaterialType MaterialType);

protected:
    // Material configurations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    TMap<EDestructionMaterialType, FDestructionMaterialConfig> MaterialConfigs;

    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    int32 MaxActiveFragments = 500;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float FragmentCleanupInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    int32 DestructionQuality = 2; // Medium by default

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    bool bEnableChainDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance Settings")
    float ChainDestructionDelay = 0.1f;

    // Runtime data
    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    int32 ActiveFragmentCount = 0;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, FDestructionMaterialConfig> RegisteredActors;

    UPROPERTY()
    TArray<TWeakObjectPtr<AGeometryCollectionActor>> ActiveGeometryCollections;

    UPROPERTY()
    TArray<TWeakObjectPtr<UFieldSystemComponent>> ActiveFieldSystems;

private:
    // Internal destruction logic
    void ExecuteDestruction(AActor* Actor, const FDestructionMaterialConfig& Config, const FDestructionEventData& EventData);
    void CreateDestructionFragments(AActor* Actor, const FDestructionMaterialConfig& Config, const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce);
    void ApplyDestructionEffects(const FDestructionEventData& EventData, const FDestructionMaterialConfig& Config);
    
    // Geometry Collection helpers
    void ConfigureGeometryCollection(AGeometryCollectionActor* GeometryActor, const FDestructionMaterialConfig& Config);
    void SetupGeometryCollectionPhysics(UGeometryCollectionComponent* GeometryComponent, const FDestructionMaterialConfig& Config);
    
    // Field system helpers
    UFieldSystemComponent* CreateFieldSystemComponent(AActor* Owner);
    void ApplyFieldToGeometryCollection(UFieldSystemComponent* FieldComponent, AGeometryCollectionActor* GeometryActor);
    
    // Performance optimization
    void OptimizeDestructionPerformance();
    void UpdateActiveFragmentCount();
    void CleanupInvalidReferences();
    
    // Event handling
    void HandleGeometryCollectionEvent(const FChaosBreakEvent& BreakEvent);
    void HandleChainDestructionTimer(AActor* Actor, float ChainRadius, float ChainForce);
    
    // Default material configurations
    void InitializeDefaultMaterialConfigs();
    FDestructionMaterialConfig CreateDefaultConfig(EDestructionMaterialType MaterialType);
    
    // Timers and delegates
    FTimerHandle CleanupTimer;
    FTimerHandle PerformanceOptimizationTimer;
    
    // Debug visualization
    bool bDebugVisualizationEnabled = false;
    void DrawDestructionDebugInfo();
    
    // System state
    bool bSystemInitialized = false;
};

/**
 * Destructible Actor Component - Attach to actors that should be destructible
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Destruction), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDestructibleActorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDestructibleActorComponent();

    // UActorComponent interface
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // Destruction configuration
    UFUNCTION(BlueprintCallable, Category = "Destructible Actor")
    void SetMaterialType(EDestructionMaterialType MaterialType);

    UFUNCTION(BlueprintPure, Category = "Destructible Actor")
    EDestructionMaterialType GetMaterialType() const { return MaterialType; }

    UFUNCTION(BlueprintCallable, Category = "Destructible Actor")
    void SetDamageThreshold(float Threshold);

    UFUNCTION(BlueprintPure, Category = "Destructible Actor")
    float GetDamageThreshold() const { return DamageThreshold; }

    // Destruction triggers
    UFUNCTION(BlueprintCallable, Category = "Destructible Actor")
    void TriggerDestruction(EDestructionTriggerType TriggerType, const FVector& ImpactLocation = FVector::ZeroVector, float ImpactForce = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Destructible Actor")
    void ApplyDamage(float Damage, const FVector& ImpactLocation = FVector::ZeroVector);

    UFUNCTION(BlueprintPure, Category = "Destructible Actor")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintPure, Category = "Destructible Actor")
    bool IsDestroyed() const { return bIsDestroyed; }

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Destructible Actor")
    FOnDestructionEvent OnActorDestruction;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    EDestructionMaterialType MaterialType = EDestructionMaterialType::Stone;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    float DamageThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    bool bAutoRegisterWithSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Destruction Settings")
    bool bEnableHealthSystem = true;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    float CurrentHealth = 100.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime Data")
    bool bIsDestroyed = false;

private:
    UPROPERTY()
    UAdvancedDestructionSystem* DestructionSystem = nullptr;

    void RegisterWithDestructionSystem();
    void UnregisterFromDestructionSystem();
    void InitializeHealthSystem();

    // Collision event handling
    UFUNCTION()
    void OnActorHit(AActor* SelfActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit);
};