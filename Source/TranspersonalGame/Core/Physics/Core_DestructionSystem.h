#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Core_DestructionSystem.generated.h"

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode"),
    Burn            UMETA(DisplayName = "Burn")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DamageThreshold = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    int32 FragmentCount = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float ImpulseStrength = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bUsePhysicsFragments = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<UParticleSystem> DestructionEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TSoftObjectPtr<USoundCue> DestructionSound;

    FCore_DestructionData()
    {
        DestructionType = ECore_DestructionType::Fracture;
        DamageThreshold = 100.0f;
        FragmentCount = 8;
        FragmentLifetime = 10.0f;
        ImpulseStrength = 500.0f;
        bUsePhysicsFragments = true;
    }
};

/**
 * Core Destruction System - Handles realistic object destruction and fragmentation
 * Supports multiple destruction types: fracture, shatter, crumble, explode, burn
 * Integrates with physics system for realistic fragment behavior
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_DestructionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_DestructionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core destruction functionality
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void TriggerDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection, float Damage);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void SetDestructionData(const FCore_DestructionData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void RepairObject();

    // Fragment management
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CreateFragments(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void CleanupFragments();

    // Destruction type handlers
    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleFractureDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleShatterDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleCrumbleDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleExplodeDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    UFUNCTION(BlueprintCallable, Category = "Destruction")
    void HandleBurnDestruction(const FVector& ImpactPoint, const FVector& ImpactDirection);

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDestructionTriggered, AActor*, DestroyedActor, ECore_DestructionType, DestructionType);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnDestructionTriggered OnDestructionTriggered;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFragmentCreated, AActor*, Fragment);
    UPROPERTY(BlueprintAssignable, Category = "Destruction")
    FOnFragmentCreated OnFragmentCreated;

protected:
    // Destruction configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bIsDestroyed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    float CurrentDamage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    bool bAutoCleanupFragments = true;

    // Fragment tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    TArray<AActor*> ActiveFragments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    float FragmentCleanupTimer = 0.0f;

    // Original mesh data for restoration
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    UStaticMesh* OriginalMesh = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Destruction", meta = (AllowPrivateAccess = "true"))
    UMaterialInterface* OriginalMaterial = nullptr;

private:
    // Internal helper functions
    void InitializeDestructionSystem();
    void UpdateFragmentLifetime(float DeltaTime);
    AActor* CreateFragment(const FVector& Location, const FVector& Velocity, float Scale);
    void ApplyFragmentPhysics(AActor* Fragment, const FVector& Velocity);
    void PlayDestructionEffects(const FVector& Location);
    UStaticMesh* GetFragmentMesh() const;
};