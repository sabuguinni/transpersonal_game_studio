#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "Core_DestructionSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_DestructionData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TArray<UStaticMesh*> DestructionStages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    TArray<FVector> FragmentVelocities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    bool bCanBeDestroyed = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction")
    float DestructionForceThreshold = 500.0f;

    FCore_DestructionData()
    {
        Health = 100.0f;
        MaxHealth = 100.0f;
        bCanBeDestroyed = true;
        DestructionForceThreshold = 500.0f;
    }
};

UENUM(BlueprintType)
enum class ECore_DestructionType : uint8
{
    None            UMETA(DisplayName = "None"),
    Fracture        UMETA(DisplayName = "Fracture"),
    Shatter         UMETA(DisplayName = "Shatter"),
    Crumble         UMETA(DisplayName = "Crumble"),
    Explode         UMETA(DisplayName = "Explode"),
    Dissolve        UMETA(DisplayName = "Dissolve")
};

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

    // Destruction Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction")
    FCore_DestructionData DestructionData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction")
    ECore_DestructionType DestructionType = ECore_DestructionType::Fracture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction")
    int32 MaxFragments = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction")
    float FragmentScaleVariation = 0.3f;

    // Visual Effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|VFX")
    UParticleSystem* DestructionParticles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|VFX")
    UMaterialInterface* FragmentMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|VFX")
    bool bSpawnDebris = true;

    // Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Audio")
    USoundBase* DestructionSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Audio")
    float SoundVolumeMultiplier = 1.0f;

    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Physics")
    float FragmentMass = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Physics")
    float ExplosionForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Physics")
    float ExplosionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Core Destruction|Physics")
    bool bUseRadialDamage = true;

    // Destruction Methods
    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void ApplyDamage(float DamageAmount, const FVector& ImpactLocation, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void TriggerDestruction(const FVector& DestructionLocation, float Force = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void CreateFragments(const FVector& Origin, int32 NumFragments);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    void SetDestructionType(ECore_DestructionType NewType);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    bool CanBeDestroyed() const;

    UFUNCTION(BlueprintCallable, Category = "Core Destruction")
    float GetHealthPercentage() const;

    // Dinosaur-Specific Destruction
    UFUNCTION(BlueprintCallable, Category = "Core Destruction|Dinosaur")
    void SetupDinosaurDestruction(float DinosaurMass, ECore_DestructionType Type);

    UFUNCTION(BlueprintCallable, Category = "Core Destruction|Dinosaur")
    void ApplyDinosaurImpact(float DinosaurMass, const FVector& ImpactVelocity);

    // Environment Destruction
    UFUNCTION(BlueprintCallable, Category = "Core Destruction|Environment")
    void SetupTreeDestruction();

    UFUNCTION(BlueprintCallable, Category = "Core Destruction|Environment")
    void SetupRockDestruction();

    UFUNCTION(BlueprintCallable, Category = "Core Destruction|Environment")
    void SetupStructureDestruction();

protected:
    // Internal Methods
    void PlayDestructionEffects(const FVector& Location);
    void CleanupFragments();
    void UpdateDestructionStage();
    UStaticMeshComponent* CreateFragment(const FVector& Location, const FVector& Velocity, float Scale);

    // Timer Handles
    FTimerHandle FragmentCleanupTimer;
    
    // Fragment Tracking
    UPROPERTY()
    TArray<UStaticMeshComponent*> SpawnedFragments;

    // State
    bool bIsDestroyed = false;
    int32 CurrentDestructionStage = 0;
};