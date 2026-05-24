#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/CollisionProfile.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Eng_BiomeArchitecture.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsObjectType : uint8
{
    Dinosaur UMETA(DisplayName = "Dinosaur"),
    Environment UMETA(DisplayName = "Environment"),
    Debris UMETA(DisplayName = "Debris"),
    Character UMETA(DisplayName = "Character"),
    Projectile UMETA(DisplayName = "Projectile")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float Mass = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableGravity = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bSimulatePhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    TEnumAsByte<ECollisionChannel> CollisionObjectType = ECC_PhysicsBody;

    FCore_PhysicsSettings()
    {
        Mass = 1000.0f;
        LinearDamping = 0.1f;
        AngularDamping = 0.1f;
        bEnableGravity = true;
        bSimulatePhysics = true;
        CollisionObjectType = ECC_PhysicsBody;
    }
};

USTRUCT(BlueprintType)
struct FCore_RagdollSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneLinearDamping = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float BoneAngularDamping = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    float RagdollLifetime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll")
    bool bAutoCleanup = true;

    FCore_RagdollSettings()
    {
        BoneLinearDamping = 0.5f;
        BoneAngularDamping = 0.5f;
        RagdollLifetime = 30.0f;
        bAutoCleanup = true;
    }
};

/**
 * Core Physics Manager - Handles realistic physics simulation for dinosaurs and environment
 * Manages collision detection, ragdoll systems, and destruction physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics setup functions
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupActorPhysics(AActor* Actor, ECore_PhysicsObjectType ObjectType);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ApplyPhysicsSettings(UStaticMeshComponent* MeshComponent, const FCore_PhysicsSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupDinosaurPhysics(AActor* DinosaurActor, float Mass = 5000.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupEnvironmentPhysics(AActor* EnvironmentActor, bool bStatic = true);

    // Ragdoll system
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdoll(AActor* Actor, const FCore_RagdollSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdoll(AActor* Actor);

    // Collision management
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetupCollisionProfile(UPrimitiveComponent* Component, const FName& ProfileName);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdateCollisionChannels(UPrimitiveComponent* Component, ECore_PhysicsObjectType ObjectType);

    // Physics testing and validation
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void CreatePhysicsTestObjects();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ValidatePhysicsSetup();

    // Biome-aware physics spawning
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    AActor* SpawnPhysicsObjectInBiome(TSubclassOf<AActor> ActorClass, EBiomeType BiomeType, const FCore_PhysicsSettings& Settings);

    // Physics cleanup
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void CleanupRagdolls();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void ResetActorPhysics(AActor* Actor);

protected:
    // Default physics settings per object type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    TMap<ECore_PhysicsObjectType, FCore_PhysicsSettings> DefaultPhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_RagdollSettings DefaultRagdollSettings;

    // Active ragdoll tracking
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ActiveRagdolls;

    // Physics validation
    UPROPERTY()
    int32 PhysicsObjectCount;

    UPROPERTY()
    int32 CollisionSetupCount;

private:
    // Internal helper functions
    void InitializeDefaultSettings();
    void SetupCollisionChannels();
    FCore_PhysicsSettings GetSettingsForObjectType(ECore_PhysicsObjectType ObjectType) const;
    void RegisterRagdoll(AActor* Actor);
    void UnregisterRagdoll(AActor* Actor);

    // Biome architecture reference
    UPROPERTY()
    TWeakObjectPtr<UEng_BiomeArchitecture> BiomeArchitecture;

    // Performance monitoring
    UPROPERTY()
    float LastPhysicsUpdateTime;

    UPROPERTY()
    int32 ActivePhysicsObjects;
};