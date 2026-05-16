#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructiblePart
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FString PartName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bIsDestroyed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    UStaticMeshComponent* MeshComponent;

    FCore_DestructiblePart()
    {
        PartName = TEXT("Unknown");
        Health = 100.0f;
        MaxHealth = 100.0f;
        bIsDestroyed = false;
        MeshComponent = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    AActor* DestroyedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Damage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    EPhysicsType DestructionType;

    FCore_DestructionEvent()
    {
        DestroyedActor = nullptr;
        ImpactLocation = FVector::ZeroVector;
        Damage = 0.0f;
        DestructionType = EPhysicsType::Static;
    }
};

/**
 * Core_DestructionSystem - Manages realistic destruction of prehistoric world objects
 * Handles tree falling, rock breaking, dinosaur bone fractures, and structure collapse
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core destruction functions
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void ApplyDamage(float Damage, const FVector& ImpactLocation, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void DestroyPart(const FString& PartName);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void CreateDebris(const FVector& Location, int32 DebrisCount);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void SimulateTreeFall(const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void SimulateRockBreak(float ImpactForce);

    // Physics-based destruction
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void EnablePhysicsDestruction(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void SetDestructionThreshold(float Threshold);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    bool IsFullyDestroyed() const;

    // Destruction event handling
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void RegisterDestructionEvent(const FCore_DestructionEvent& Event);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    TArray<FCore_DestructionEvent> GetRecentDestructionEvents() const;

protected:
    // Destructible parts management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_DestructiblePart> DestructibleParts;

    // Destruction settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    float DestructionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    bool bPhysicsDestructionEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    float DebrisLifetime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    int32 MaxDebrisCount;

    // Event tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_DestructionEvent> DestructionEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    int32 MaxEventHistory;

    // Performance optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction", meta = (AllowPrivateAccess = "true"))
    float LastUpdateTime;

private:
    void InitializeDestructibleParts();
    void UpdateDestructionStates(float DeltaTime);
    void CleanupOldEvents();
    void SpawnDebrisActor(const FVector& Location, const FVector& Velocity);
    float CalculateDamageFromImpact(float ImpactForce) const;
};