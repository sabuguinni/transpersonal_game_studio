#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Core_DestructionSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDestructionSystem, Log, All);

/**
 * Destruction Types for different destruction scenarios
 */
UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "No Destruction"),
    Simple          UMETA(DisplayName = "Simple Break"),
    Shatter         UMETA(DisplayName = "Shatter Into Pieces"),
    Crumble         UMETA(DisplayName = "Crumble Down"),
    Explode         UMETA(DisplayName = "Explosive Destruction"),
    Burn            UMETA(DisplayName = "Fire Destruction")
};

/**
 * Destruction Material Types
 */
UENUM(BlueprintType)
enum class ECore_MaterialType : uint8
{
    Wood            UMETA(DisplayName = "Wood"),
    Stone           UMETA(DisplayName = "Stone"),
    Metal           UMETA(DisplayName = "Metal"),
    Glass           UMETA(DisplayName = "Glass"),
    Bone            UMETA(DisplayName = "Bone"),
    Flesh           UMETA(DisplayName = "Flesh"),
    Ice             UMETA(DisplayName = "Ice"),
    Crystal         UMETA(DisplayName = "Crystal")
};

/**
 * Destruction Event Data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FVector ImpactLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    FVector ImpactDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpactForce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    AActor* InstigatorActor;

    FCore_DestructionEvent()
    {
        DestructionType = ECore_DestructionType::None;
        ImpactLocation = FVector::ZeroVector;
        ImpactDirection = FVector::ForwardVector;
        ImpactForce = 0.0f;
        InstigatorActor = nullptr;
    }
};

/**
 * Core Destruction System Component
 * 
 * Handles realistic destruction of objects in the prehistoric world.
 * Supports different material types and destruction patterns.
 * Integrates with physics system for realistic debris behavior.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    ECore_MaterialType MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float DestructionThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    bool bCanBeDestroyed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    bool bCreateDebris;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    int32 MaxDebrisCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Settings")
    float DebrisLifetime;

    // Destruction Meshes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Meshes")
    TArray<UStaticMesh*> DebrisMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Meshes")
    UStaticMesh* DestroyedMesh;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DebrisMass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DebrisBounciness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float DebrisFriction;

    // Sound and Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* DestructionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UParticleSystem* DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    class UNiagaraSystem* DestructionNiagara;

    // Destruction Functions
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TakeDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactDirection, AActor* DamageCauser);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void DestroyObject(const FCore_DestructionEvent& DestructionEvent);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateDebris(const FVector& ImpactLocation, const FVector& ImpactDirection, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void PlayDestructionEffects(const FVector& Location);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Destruction")
    float GetHealthPercentage() const;

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool IsDestroyed() const;

    UFUNCTION(BlueprintPure, Category = "Destruction")
    bool CanTakeDamage() const;

    // Material-specific destruction patterns
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void ApplyMaterialSpecificDestruction(ECore_MaterialType Material, const FCore_DestructionEvent& Event);

protected:
    // Internal Functions
    void InitializeDestructionSystem();
    void CleanupDebris();
    FVector CalculateDebrisVelocity(const FVector& ImpactDirection, float Force) const;
    void SpawnDebrisActor(const FVector& Location, const FVector& Velocity, UStaticMesh* DebrisMesh);

private:
    // Internal State
    bool bIsDestroyed;
    TArray<AActor*> SpawnedDebris;
    float DestructionTime;

    // Cleanup timer
    FTimerHandle DebrisCleanupTimer;
};