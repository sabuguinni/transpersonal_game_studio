#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "SharedTypes.h"
#include "Eng_PhysicsSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gravity")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Gravity")
    FVector WorldGravity = FVector(0.0f, 0.0f, -980.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Simulation")
    float PhysicsTickRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Simulation")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Collision")
    bool bEnableComplexCollision = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Collision")
    float CollisionTolerance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Performance")
    int32 MaxPhysicsBodies = 5000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Performance")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll")
    bool bEnableRagdollPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Ragdoll")
    float RagdollBlendTime = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Destruction")
    bool bEnableDestruction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics|Destruction")
    float DestructionThreshold = 100.0f;

    FEng_PhysicsConfiguration()
    {
        // Default values already set above
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics Configuration Management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void InitializePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdatePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetWorldGravity(const FVector& NewGravity);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FEng_PhysicsConfiguration GetPhysicsConfiguration() const { return PhysicsConfig; }

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsConfiguration(const FEng_PhysicsConfiguration& NewConfig);

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    void EnableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    void DisableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Ragdoll")
    bool IsActorInRagdoll(AActor* Actor) const;

    // Destruction System
    UFUNCTION(BlueprintCallable, Category = "Physics System|Destruction")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Physics System|Destruction")
    void SetDestructionThreshold(float NewThreshold);

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Physics System|Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics System|Performance")
    int32 GetActivePhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Physics System|Performance")
    void CullDistantPhysicsBodies();

    // Debug and Diagnostics
    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug", CallInEditor)
    void DebugPhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug")
    void LogPhysicsStats();

    UFUNCTION(BlueprintCallable, Category = "Physics System|Debug")
    void DrawPhysicsDebugInfo(bool bEnabled);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Configuration", meta = (AllowPrivateAccess = "true"))
    FEng_PhysicsConfiguration PhysicsConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    bool bIsInitialized = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<AActor>> RagdollActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State", meta = (AllowPrivateAccess = "true"))
    TArray<TWeakObjectPtr<AActor>> DestructibleActors;

private:
    void ApplyPhysicsSettings();
    void CleanupWeakReferences();
    void ValidatePhysicsConfiguration();
};