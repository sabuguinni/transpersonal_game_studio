#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "PhysicsDebugComponent.generated.h"

/** Physics debug visualization types */
UENUM(BlueprintType)
enum class EPhysicsDebugType : uint8
{
    CollisionShapes     UMETA(DisplayName = "Collision Shapes"),
    ContactPoints       UMETA(DisplayName = "Contact Points"),
    ForceVectors        UMETA(DisplayName = "Force Vectors"),
    VelocityVectors     UMETA(DisplayName = "Velocity Vectors"),
    CenterOfMass        UMETA(DisplayName = "Center of Mass"),
    BoundingBoxes       UMETA(DisplayName = "Bounding Boxes"),
    PhysicsConstraints  UMETA(DisplayName = "Physics Constraints"),
    All                 UMETA(DisplayName = "All Debug Info")
};

/** Debug draw settings */
USTRUCT(BlueprintType)
struct FPhysicsDebugSettings
{
    GENERATED_BODY()

    /** Enable debug drawing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bEnabled = false;

    /** Debug draw color */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FColor DrawColor = FColor::Green;

    /** Line thickness for debug drawing */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float LineThickness = 1.0f;

    /** Duration to keep debug lines visible */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "60.0"))
    float DrawDuration = 0.0f;

    /** Scale factor for vector visualization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1", ClampMax = "100.0"))
    float VectorScale = 1.0f;
};

/**
 * @brief Physics debug visualization component for development and testing
 * 
 * Provides comprehensive debug visualization for:
 * - Collision shapes and contact points
 * - Force and velocity vectors
 * - Center of mass and inertia
 * - Physics constraints and joints
 * - Performance metrics overlay
 * 
 * Essential for debugging physics interactions in the prehistoric world,
 * especially for complex dinosaur behaviors and environmental destruction.
 * 
 * @author Core Systems Programmer — Agent #3
 * @version 1.0 — March 2026
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsDebugComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsDebugComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /**
     * @brief Enable/disable physics debug visualization
     * 
     * @param DebugType Type of debug visualization to toggle
     * @param bEnabled Whether to enable or disable
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void SetDebugVisualization(EPhysicsDebugType DebugType, bool bEnabled);

    /**
     * @brief Enable all debug visualizations
     * 
     * @param bEnabled Whether to enable or disable all debug types
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void SetAllDebugVisualization(bool bEnabled);

    /**
     * @brief Configure debug draw settings for a specific type
     * 
     * @param DebugType Type of debug visualization to configure
     * @param Settings Debug draw settings to apply
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ConfigureDebugSettings(EPhysicsDebugType DebugType, const FPhysicsDebugSettings& Settings);

    /**
     * @brief Draw collision shapes for an actor
     * 
     * @param TargetActor Actor to draw collision shapes for
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawCollisionShapes(AActor* TargetActor, FColor Color = FColor::Green, float Duration = 0.0f);

    /**
     * @brief Draw contact points between two actors
     * 
     * @param ActorA First actor in contact
     * @param ActorB Second actor in contact
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawContactPoints(AActor* ActorA, AActor* ActorB, FColor Color = FColor::Red, float Duration = 0.0f);

    /**
     * @brief Draw force vectors acting on an actor
     * 
     * @param TargetActor Actor to draw forces for
     * @param Scale Scale factor for force visualization
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawForceVectors(AActor* TargetActor, float Scale = 1.0f, FColor Color = FColor::Blue, float Duration = 0.0f);

    /**
     * @brief Draw velocity vectors for an actor
     * 
     * @param TargetActor Actor to draw velocity for
     * @param Scale Scale factor for velocity visualization
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawVelocityVectors(AActor* TargetActor, float Scale = 1.0f, FColor Color = FColor::Yellow, float Duration = 0.0f);

    /**
     * @brief Draw center of mass for an actor
     * 
     * @param TargetActor Actor to draw center of mass for
     * @param Size Size of the center of mass indicator
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawCenterOfMass(AActor* TargetActor, float Size = 10.0f, FColor Color = FColor::Magenta, float Duration = 0.0f);

    /**
     * @brief Draw physics constraints for an actor
     * 
     * @param TargetActor Actor to draw constraints for
     * @param Color Color to use for drawing
     * @param Duration How long to keep the debug lines visible
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawPhysicsConstraints(AActor* TargetActor, FColor Color, float Duration = 0.0f);

    /**
     * @brief Draw performance metrics overlay
     * 
     * @param bShowFrameTime Whether to show frame time
     * @param bShowMemoryUsage Whether to show memory usage
     * @param bShowBodyCount Whether to show active body count
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawPerformanceOverlay(bool bShowFrameTime = true, bool bShowMemoryUsage = true, bool bShowBodyCount = true);

    /**
     * @brief Set debug draw distance (objects beyond this distance won't be drawn)
     * 
     * @param Distance Maximum distance for debug drawing
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void SetDebugDrawDistance(float Distance);

    /**
     * @brief Take a physics debug screenshot
     * 
     * @param Filename Name of the screenshot file
     * @param bIncludeUI Whether to include debug UI in screenshot
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void TakeDebugScreenshot(const FString& Filename, bool bIncludeUI = true);

protected:
    /** Debug settings for each visualization type */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Settings")
    TMap<EPhysicsDebugType, FPhysicsDebugSettings> DebugSettings;

    /** Maximum distance for debug drawing */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "100.0", ClampMax = "50000.0"))
    float MaxDebugDrawDistance = 5000.0f;

    /** Enable performance overlay */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug Settings")
    bool bShowPerformanceOverlay = false;

    /** Update frequency for performance metrics (in seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance", meta = (ClampMin = "0.1", ClampMax = "5.0"))
    float PerformanceUpdateInterval = 1.0f;

private:
    /** Draw collision shapes for a primitive component */
    void DrawCollisionShapeForComponent(UPrimitiveComponent* Component, const FPhysicsDebugSettings& Settings);
    
    /** Draw bounding box for an actor */
    void DrawBoundingBox(AActor* Actor, const FPhysicsDebugSettings& Settings);
    
    /** Check if actor is within debug draw distance */
    bool IsActorInDebugRange(AActor* Actor) const;
    
    /** Get player location for distance calculations */
    FVector GetPlayerLocation() const;
    
    /** Update performance metrics */
    void UpdatePerformanceMetrics();
    
    /** Draw performance text on screen */
    void DrawPerformanceText();

    /** Cached world reference */
    UPROPERTY()
    UWorld* CachedWorld;

    /** Performance metrics */
    float CurrentFrameTime = 0.0f;
    float CurrentMemoryUsage = 0.0f;
    int32 CurrentActiveBodyCount = 0;
    
    /** Performance update timer */
    FTimerHandle PerformanceUpdateTimer;
    
    /** Frame time history for smoothing */
    TArray<float> FrameTimeHistory;
    
    /** Debug draw enabled state for each type */
    TMap<EPhysicsDebugType, bool> DebugEnabled;
};