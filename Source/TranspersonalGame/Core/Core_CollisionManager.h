#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Core_CollisionManager.generated.h"

/**
 * Core Collision Manager - Handles advanced collision detection and response
 * for realistic physics interactions in the prehistoric survival world
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_CollisionManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_CollisionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Collision Detection Methods
    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    bool PerformAdvancedCollisionCheck(AActor* ActorA, AActor* ActorB, FHitResult& OutHitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void HandleDinosaurCollision(AActor* Dinosaur, const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void HandlePlayerCollision(AActor* Player, const FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Physics")
    void HandleEnvironmentCollision(AActor* EnvironmentActor, const FHitResult& HitResult);

    // Collision Response Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float CollisionDamageMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    float MinimumImpactVelocity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    bool bEnableAdvancedCollisionLogging = true;

    // Material-based collision responses
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Physics")
    TMap<TEnumAsByte<EPhysicalSurface>, float> SurfaceResponseMultipliers;

protected:
    // Internal collision processing
    void ProcessCollisionDamage(AActor* Actor, float ImpactForce);
    void ApplyCollisionEffects(const FHitResult& HitResult, float ImpactForce);
    
    UPROPERTY()
    TArray<AActor*> TrackedActors;

    UPROPERTY()
    float LastCollisionTime = 0.0f;
};