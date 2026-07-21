#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "Subsystems/WorldSubsystem.h"
#include "Core_CollisionManager.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Terrain         UMETA(DisplayName = "Terrain"),
    Character       UMETA(DisplayName = "Character"),
    Dinosaur        UMETA(DisplayName = "Dinosaur"),
    Vegetation      UMETA(DisplayName = "Vegetation"),
    Structure       UMETA(DisplayName = "Structure"),
    Projectile      UMETA(DisplayName = "Projectile"),
    Water           UMETA(DisplayName = "Water"),
    Lava            UMETA(DisplayName = "Lava")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    ECore_CollisionType CollisionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Mass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Friction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    float Restitution;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanBeDestroyed;

    FCore_CollisionData()
    {
        CollisionType = ECore_CollisionType::None;
        Mass = 1.0f;
        Friction = 0.7f;
        Restitution = 0.3f;
        bCanBeDestroyed = false;
    }
};

/**
 * Core Collision Manager - Handles all collision detection and response in the prehistoric world
 * Manages terrain interaction, character-dinosaur collisions, and environmental physics
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_CollisionManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_CollisionManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Collision Registration
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void RegisterCollisionObject(UPrimitiveComponent* Component, const FCore_CollisionData& CollisionData);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void UnregisterCollisionObject(UPrimitiveComponent* Component);

    // Collision Queries
    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool LineTraceForTerrain(const FVector& Start, const FVector& End, FHitResult& OutHit);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool SphereTraceForDinosaurs(const FVector& Center, float Radius, TArray<FHitResult>& OutHits);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    bool CheckGroundStability(const FVector& Location, float& OutStabilityFactor);

    // Physics Material Management
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ApplyTerrainPhysics(UPrimitiveComponent* Component, ECore_CollisionType TerrainType);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void SetCollisionResponseForDinosaur(UPrimitiveComponent* Component, bool bIsLargeDinosaur);

    // Impact Events
    UFUNCTION(BlueprintCallable, Category = "Collision")
    void HandleImpactEvent(const FHitResult& Hit, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Collision")
    void ProcessDestructibleCollision(UPrimitiveComponent* Component, const FVector& ImpactPoint, float Force);

protected:
    // Registered collision objects
    UPROPERTY()
    TMap<UPrimitiveComponent*, FCore_CollisionData> CollisionRegistry;

    // Physics materials for different terrain types
    UPROPERTY(EditAnywhere, Category = "Physics Materials")
    TMap<ECore_CollisionType, class UPhysicalMaterial*> TerrainMaterials;

    // Collision channels
    UPROPERTY(EditAnywhere, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> TerrainChannel;

    UPROPERTY(EditAnywhere, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> DinosaurChannel;

    UPROPERTY(EditAnywhere, Category = "Collision Channels")
    TEnumAsByte<ECollisionChannel> CharacterChannel;

private:
    void InitializePhysicsMaterials();
    void SetupCollisionChannels();
    float CalculateImpactDamage(const FHitResult& Hit, float Force);
};