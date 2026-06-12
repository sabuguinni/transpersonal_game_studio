#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core_PhysicsSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

UENUM(BlueprintType)
enum class ECore_TerrainType : uint8
{
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Sand        UMETA(DisplayName = "Sand"),
    Mud         UMETA(DisplayName = "Mud"),
    Snow        UMETA(DisplayName = "Snow"),
    Water       UMETA(DisplayName = "Water"),
    Lava        UMETA(DisplayName = "Lava")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxPhysicsDeltaTime = 0.033f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsQuality PhysicsQuality = ECore_PhysicsQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableRagdollPhysics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    bool bEnableTerrainInteraction = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_TerrainPhysicsData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ECore_TerrainType TerrainType = ECore_TerrainType::Grass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Friction = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Restitution = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float Density = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bAffectsMovement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float MovementSpeedMultiplier = 1.0f;
};

/**
 * Core Physics System Manager
 * Manages physics settings, terrain interaction, and ragdoll systems
 * Provides centralized physics configuration for the prehistoric survival game
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics Configuration
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void InitializePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetPhysicsQuality(ECore_PhysicsQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void SetGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UpdatePhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    // Terrain Physics
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void RegisterTerrainActor(AActor* TerrainActor, const FCore_TerrainPhysicsData& PhysicsData);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void UnregisterTerrainActor(AActor* TerrainActor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    FCore_TerrainPhysicsData GetTerrainPhysicsData(AActor* TerrainActor) const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    ECore_TerrainType GetTerrainTypeAtLocation(const FVector& WorldLocation) const;

    // Ragdoll System
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void EnableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void DisableRagdollForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool IsRagdollEnabled(AActor* Actor) const;

    // Physics Validation
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool ValidatePhysicsSetup() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    TArray<FString> GetPhysicsValidationErrors() const;

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    float GetCurrentPhysicsFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    int32 GetActivePhysicsBodies() const;

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void OptimizePhysicsPerformance();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    FCore_PhysicsSettings CurrentPhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    TMap<AActor*, FCore_TerrainPhysicsData> TerrainPhysicsMap;

    UPROPERTY(BlueprintReadOnly, Category = "Core Physics", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> RagdollEnabledActors;

private:
    void ApplyPhysicsSettings();
    void UpdateTerrainPhysicsMaterials();
    void MonitorPhysicsPerformance();
    
    UPROPERTY()
    class UPhysicsSettings* CachedPhysicsSettings;
    
    float LastPhysicsFrameTime;
    int32 LastPhysicsBodiesCount;
};