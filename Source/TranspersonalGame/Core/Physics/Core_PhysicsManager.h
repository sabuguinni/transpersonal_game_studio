#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsMode : uint8
{
    Realistic       UMETA(DisplayName = "Realistic Physics"),
    Arcade          UMETA(DisplayName = "Arcade Physics"),
    Cinematic       UMETA(DisplayName = "Cinematic Physics")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float Gravity = -980.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float AngularDamping = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float MaxAngularVelocity = 3600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    bool bEnableAsyncScene = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float BounceCombineMode = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Profile")
    float FrictionCombineMode = 0.5f;
};

/**
 * Core Physics Manager - Handles global physics settings and validation
 * Manages realistic physics simulation for prehistoric survival gameplay
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === PHYSICS CONFIGURATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    ECore_PhysicsMode PhysicsMode = ECore_PhysicsMode::Realistic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsProfile CurrentProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsValidation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float ValidationInterval = 1.0f;

    // === PHYSICS MATERIALS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Materials")
    TMap<FString, class UPhysicalMaterial*> PhysicsMaterials;

    // === COLLISION SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bEnableComplexCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float CollisionTolerance = 0.1f;

    // === PERFORMANCE SETTINGS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxPhysicsObjects = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PhysicsLODDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnablePhysicsLOD = true;

    // === METHODS ===
    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void ApplyPhysicsProfile(const FCore_PhysicsProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void SetPhysicsMode(ECore_PhysicsMode NewMode);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    bool ValidatePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void RegisterPhysicsMaterial(const FString& MaterialName, UPhysicalMaterial* Material);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    UPhysicalMaterial* GetPhysicsMaterial(const FString& MaterialName);

    UFUNCTION(BlueprintCallable, Category = "Physics Management")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Management", CallInEditor = true)
    void DebugPhysicsState();

private:
    // === INTERNAL STATE ===
    float LastValidationTime = 0.0f;
    int32 CurrentPhysicsObjectCount = 0;
    
    // === INTERNAL METHODS ===
    void UpdatePhysicsObjectCount();
    void ApplyPerformanceOptimizations();
    void ValidatePhysicsObjects();
    void LogPhysicsStatistics();
};