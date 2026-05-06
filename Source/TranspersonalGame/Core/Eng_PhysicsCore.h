#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_PhysicsCore.generated.h"

UENUM(BlueprintType)
enum class EEng_PhysicsProfile : uint8
{
    Default         UMETA(DisplayName = "Default Physics"),
    Dinosaur        UMETA(DisplayName = "Dinosaur Physics"),
    Character       UMETA(DisplayName = "Character Physics"),
    Projectile      UMETA(DisplayName = "Projectile Physics"),
    Environment     UMETA(DisplayName = "Environment Physics")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PhysicsProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Friction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Restitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics;

    FEng_PhysicsProfile()
    {
        Mass = 100.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.01f;
        Friction = 0.7f;
        Restitution = 0.3f;
        bEnableGravity = true;
        bSimulatePhysics = true;
    }
};

/**
 * Engine Architecture - Physics Core System
 * Manages physics profiles, collision channels, and performance optimization
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_PhysicsCore : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_PhysicsCore();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics profile management
    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    FEng_PhysicsProfile GetPhysicsProfile(EEng_PhysicsProfile ProfileType) const;

    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void ApplyPhysicsProfile(UPrimitiveComponent* Component, EEng_PhysicsProfile ProfileType);

    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void SetGlobalPhysicsSettings();

    // Collision management
    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void ConfigureCollisionChannels();

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void OptimizePhysicsForBiome(const FVector& Location);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Engine Physics")
    void ValidatePhysicsSetup();

    // Ragdoll and destruction
    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void EnableRagdoll(USkeletalMeshComponent* SkeletalMesh);

    UFUNCTION(BlueprintCallable, Category = "Engine Physics")
    void CreateDestructibleMesh(UStaticMeshComponent* StaticMesh, float ImpactThreshold);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Configuration")
    TMap<EEng_PhysicsProfile, FEng_PhysicsProfile> PhysicsProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Configuration")
    float PhysicsSimulationDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Physics Configuration")
    int32 MaxPhysicsObjects;

    void InitializePhysicsProfiles();
    void SetupCollisionChannels();
    void OptimizePhysicsSettings();
};