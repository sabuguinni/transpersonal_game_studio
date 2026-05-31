#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bAutoCleanup = true;

    FCore_DestructionData()
    {
        DestructionType = ECore_DestructionType::Fracture;
        DestructionThreshold = 100.0f;
        FragmentLifetime = 10.0f;
        MaxFragments = 20;
        bAutoCleanup = true;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core destruction properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float CurrentDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    TArray<UStaticMesh*> FragmentMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    UMaterialInterface* FragmentMaterial;

    // Destruction state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    bool bIsDestroyed = false;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    TArray<AActor*> SpawnedFragments;

    UPROPERTY(BlueprintReadOnly, Category = "Destruction State")
    float TimeSinceDestruction = 0.0f;

    // Core destruction functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactPoint = FVector::ZeroVector, const FVector& ImpactNormal = FVector::UpVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactPoint = FVector::ZeroVector, const FVector& ImpactForce = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    float GetDestructionProgress() const;

    // Fragment management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SpawnFragments(const FVector& ImpactPoint, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    AActor* CreateFragment(UStaticMesh* FragmentMesh, const FVector& Location, const FRotator& Rotation, const FVector& Velocity);

    // Destruction validation
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool ValidateDestructionSetup() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void InitializeDestruction();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, FVector, ImpactPoint);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentSpawned, AActor*, Fragment);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDestructionComplete);

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDestructionTriggered OnDestructionTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnFragmentSpawned OnFragmentSpawned;

    UPROPERTY(BlueprintAssignable, Category = "Destruction Events")
    FOnDestructionComplete OnDestructionComplete;

private:
    // Internal state
    bool bInitialized = false;
    float FragmentCleanupTimer = 0.0f;
    
    // Helper functions
    void UpdateFragmentCleanup(float DeltaTime);
    FVector CalculateFragmentVelocity(const FVector& ImpactPoint, const FVector& FragmentLocation, const FVector& ImpactForce) const;
    void SetupFragmentPhysics(AActor* Fragment, const FVector& Velocity) const;
};