#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "../Engine/Eng_GameplayFoundation.h"
#include "Core_PhysicsValidation.h"
#include "Core_PhysicsIntegrationSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsIntegration, Log, All);

/**
 * Physics Integration Event Types
 */
UENUM(BlueprintType)
enum class ECore_PhysicsIntegrationType : uint8
{
    None                UMETA(DisplayName = "None"),
    CharacterMovement   UMETA(DisplayName = "Character Movement"),
    VehiclePhysics      UMETA(DisplayName = "Vehicle Physics"),
    DestructionSystem   UMETA(DisplayName = "Destruction System"),
    FluidDynamics       UMETA(DisplayName = "Fluid Dynamics"),
    TerrainInteraction  UMETA(DisplayName = "Terrain Interaction"),
    WeatherEffects      UMETA(DisplayName = "Weather Effects"),
    CollisionResponse   UMETA(DisplayName = "Collision Response")
};

/**
 * Physics Integration Event Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsIntegrationEvent
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    ECore_PhysicsIntegrationType EventType = ECore_PhysicsIntegrationType::None;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    float Magnitude = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    FDateTime Timestamp = FDateTime::Now();

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    TWeakObjectPtr<AActor> SourceActor = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Integration")
    FString EventDescription;
};

/**
 * Physics Integration Configuration
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsIntegrationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableCharacterMovementIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableVehiclePhysicsIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableDestructionIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableFluidDynamicsIntegration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableTerrainIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    bool bEnableWeatherIntegration = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Settings")
    float IntegrationUpdateRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimultaneousEvents = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float EventHistoryDuration = 30.0f;
};

/**
 * Physics Integration Component
 * Handles integration between different physics systems and gameplay foundation
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Integration Management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void RegisterPhysicsEvent(const FCore_PhysicsIntegrationEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetIntegrationConfig(const FCore_PhysicsIntegrationConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    FCore_PhysicsIntegrationConfig GetIntegrationConfig() const { return IntegrationConfig; }

    // Event Queries
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    TArray<FCore_PhysicsIntegrationEvent> GetRecentEvents(ECore_PhysicsIntegrationType EventType = ECore_PhysicsIntegrationType::None) const;

    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    int32 GetEventCount(ECore_PhysicsIntegrationType EventType = ECore_PhysicsIntegrationType::None) const;

    // Integration Status
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    bool IsIntegrationActive(ECore_PhysicsIntegrationType IntegrationType) const;

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetIntegrationActive(ECore_PhysicsIntegrationType IntegrationType, bool bActive);

    // Gameplay Foundation Integration
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void UpdateGameplayFoundationMetrics();

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsIntegrationConfig IntegrationConfig;

    // Event Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Events", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsIntegrationEvent> EventHistory;

    // System References
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UEng_GameplayFoundationSubsystem> GameplayFoundation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UCore_PhysicsValidation> PhysicsValidation;

private:
    // Internal Management
    void CleanupOldEvents();
    void ProcessIntegrationEvents();
    void UpdateIntegrationMetrics();
    
    // Timers
    FTimerHandle IntegrationUpdateTimer;
    FTimerHandle EventCleanupTimer;
    
    // Performance Tracking
    float LastUpdateTime;
    int32 EventsProcessedThisFrame;
};

/**
 * Physics Integration World Subsystem
 * Manages global physics integration across all actors
 */
UCLASS()
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Global Integration Management
    UFUNCTION(BlueprintCallable, Category = "Physics Integration", CallInEditor = true)
    void RegisterGlobalPhysicsEvent(const FCore_PhysicsIntegrationEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void SetGlobalIntegrationConfig(const FCore_PhysicsIntegrationConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    FCore_PhysicsIntegrationConfig GetGlobalIntegrationConfig() const { return GlobalConfig; }

    // System Queries
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    TArray<UCore_PhysicsIntegrationComponent*> GetAllIntegrationComponents() const;

    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    int32 GetTotalActiveIntegrations() const;

    // Performance Monitoring
    UFUNCTION(BlueprintPure, Category = "Physics Integration")
    float GetIntegrationPerformanceMetric() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    void OptimizeIntegrationPerformance();

protected:
    // Global Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Configuration")
    FCore_PhysicsIntegrationConfig GlobalConfig;

    // Global Event Storage
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Global Events")
    TArray<FCore_PhysicsIntegrationEvent> GlobalEventHistory;

    // Registered Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<TWeakObjectPtr<UCore_PhysicsIntegrationComponent>> RegisteredComponents;

private:
    // Internal Management
    void UpdateGlobalMetrics();
    void CleanupGlobalEvents();
    void ValidateIntegrationSystems();
    
    // Performance Tracking
    float GlobalPerformanceMetric;
    FDateTime LastGlobalUpdate;
    
    // Timers
    FTimerHandle GlobalUpdateTimer;
    FTimerHandle GlobalCleanupTimer;
};

#include "Core_PhysicsIntegrationSystem.generated.h"