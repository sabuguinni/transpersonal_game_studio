#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"
#include "SharedTypes.h"
#include "Core_CollisionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_CollisionType : uint8
{
    Character       UMETA(DisplayName = "Character Collision"),
    Dinosaur        UMETA(DisplayName = "Dinosaur Collision"),
    Environment     UMETA(DisplayName = "Environment Collision"),
    Projectile      UMETA(DisplayName = "Projectile Collision"),
    Trigger         UMETA(DisplayName = "Trigger Collision"),
    Water           UMETA(DisplayName = "Water Collision")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> ObjectType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<TEnumAsByte<ECollisionResponse>> ResponseToChannels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bGenerateOverlapEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    bool bCanCharacterStepUpOn;

    FCore_CollisionProfile()
    {
        CollisionEnabled = ECollisionEnabled::QueryAndPhysics;
        ObjectType = ECC_WorldStatic;
        bGenerateOverlapEvents = false;
        bCanCharacterStepUpOn = true;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCore_OnCollisionHit, AActor*, HitActor, AActor*, OtherActor, const FHitResult&, HitResult);

/**
 * Core Systems - Collision Management System
 * Handles collision detection, response, and optimization for all game objects
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UCore_CollisionSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_CollisionSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Collision profile management
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    FCore_CollisionProfile GetCollisionProfile(ECore_CollisionType CollisionType) const;

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void ApplyCollisionProfile(UPrimitiveComponent* Component, ECore_CollisionType CollisionType);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void SetupCollisionChannels();

    // Collision detection utilities
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool LineTraceByChannel(const FVector& Start, const FVector& End, ECollisionChannel TraceChannel, 
                           FHitResult& HitResult, bool bTraceComplex = false);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    bool SphereTraceByChannel(const FVector& Start, const FVector& End, float Radius, 
                             ECollisionChannel TraceChannel, FHitResult& HitResult);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    TArray<AActor*> GetOverlappingActors(const FVector& Location, float Radius, 
                                        ECollisionChannel QueryChannel);

    // Collision response management
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void RegisterCollisionCallback(AActor* Actor, const FCore_OnCollisionHit& Callback);

    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void HandleCollisionHit(AActor* HitActor, AActor* OtherActor, const FHitResult& HitResult);

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Core Collision")
    void OptimizeCollisionForDistance(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Core Collision")
    void ValidateCollisionSetup();

    // Collision events
    UPROPERTY(BlueprintAssignable, Category = "Core Collision")
    FCore_OnCollisionHit OnCollisionHit;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Configuration")
    TMap<ECore_CollisionType, FCore_CollisionProfile> CollisionProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Configuration")
    float MaxCollisionDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Configuration")
    int32 MaxCollisionObjects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Collision Configuration")
    bool bEnableCollisionOptimization;

    void InitializeCollisionProfiles();
    void SetupCustomCollisionChannels();
    void OptimizeCollisionSettings();

private:
    TMap<AActor*, FCore_OnCollisionHit> CollisionCallbacks;
    TArray<TWeakObjectPtr<AActor>> TrackedActors;
};