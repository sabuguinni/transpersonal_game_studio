#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/LandscapeProxy.h"
#include "Kismet/GameplayStatics.h"
#include "SharedTypes.h"
#include "Core_PhysicsArchitect.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsValidationData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bCharacterPhysicsValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bMovementComponentValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bCollisionSystemValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bTerrainInteractionValid = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bPhysicsSceneActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    float ValidationScore = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    FString ValidationStatus = TEXT("Not Validated");

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    TArray<FString> ValidationErrors;

    FCore_PhysicsValidationData()
    {
        bCharacterPhysicsValid = false;
        bMovementComponentValid = false;
        bCollisionSystemValid = false;
        bTerrainInteractionValid = false;
        bPhysicsSceneActive = false;
        ValidationScore = 0.0f;
        ValidationStatus = TEXT("Not Validated");
        ValidationErrors.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 ActiveCollisionShapes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    float CollisionQueryTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    int32 CollisionEvents = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    bool bPerformanceWithinLimits = true;

    FCore_PhysicsMetrics()
    {
        ActiveRigidBodies = 0;
        ActiveCollisionShapes = 0;
        PhysicsFrameTime = 0.0f;
        CollisionQueryTime = 0.0f;
        CollisionEvents = 0;
        bPerformanceWithinLimits = true;
    }
};

/**
 * Core Physics Architect Component
 * Validates and monitors all physics systems integration for the WALK AROUND milestone
 * Ensures character movement, collision, terrain interaction, and performance requirements
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsArchitect : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsArchitect();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Physics validation methods
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FCore_PhysicsValidationData ValidatePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateCharacterPhysics(class ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateMovementComponent(class UCharacterMovementComponent* MovementComp);

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateCollisionSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FCore_PhysicsMetrics GetPhysicsMetrics();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPhysicsPerformanceAcceptable();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float CalculatePhysicsScore();

    // Integration with Engine Architect
    UFUNCTION(BlueprintCallable, Category = "Architecture Integration")
    void ReportToEngineArchitect();

    UFUNCTION(BlueprintCallable, Category = "Architecture Integration")
    bool IsWalkAroundMilestoneReady();

protected:
    // Validation data
    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    FCore_PhysicsValidationData CurrentValidationData;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Metrics")
    FCore_PhysicsMetrics CurrentMetrics;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MaxPhysicsFrameTime = 16.67f; // 60 FPS target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MaxCollisionQueryTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxActiveRigidBodies = 1000;

    // Validation settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Validation")
    bool bAutoValidateOnTick = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Validation")
    float ValidationInterval = 1.0f;

private:
    float LastValidationTime = 0.0f;
    
    // Internal validation helpers
    bool ValidatePhysicsScene();
    bool ValidateLandscapeCollision();
    void UpdateMetrics();
    void LogValidationResults();
};