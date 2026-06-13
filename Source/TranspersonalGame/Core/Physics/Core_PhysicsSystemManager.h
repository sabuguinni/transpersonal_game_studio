#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Engine/CollisionProfile.h"
#include "Core/Shared/SharedTypes.h"
#include "Core_PhysicsSystemManager.generated.h"

// Forward declarations
class ACharacter;
class UPrimitiveComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;

/**
 * Physics configuration settings for different gameplay scenarios
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSettings
{
    GENERATED_BODY()

    // Movement physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Physics")
    float AirControl = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Physics")
    float GroundFriction = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Physics")
    float BrakingFriction = 2.0f;

    // Collision physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float CollisionResponseMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float ImpactDamageThreshold = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision Physics")
    float MaxImpactDamage = 100.0f;

    // Ragdoll physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollActivationForce = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    float RagdollDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ragdoll Physics")
    bool bAutoRecoverFromRagdoll = true;

    // Destruction physics
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float DestructionThreshold = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    float FragmentLifetime = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destruction Physics")
    int32 MaxFragments = 20;

    FCore_PhysicsSettings()
    {
        // Default constructor with reasonable values
    }
};

/**
 * Physics state tracking for dynamic adjustments
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    bool bIsRagdollActive = false;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float CurrentGravityScale = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float CurrentFriction = 8.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float LastImpactForce = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    float RagdollTimeRemaining = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    int32 ActiveFragments = 0;

    FCore_PhysicsState()
    {
        // Default constructor
    }
};

/**
 * Core Physics System Manager
 * Manages all physics-related systems including movement, collision, ragdoll, and destruction
 * Integrates with survival systems to modify physics behavior based on character state
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics settings management
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void SetPhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsSettings GetPhysicsSettings() const { return PhysicsSettings; }

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ResetToDefaultSettings();

    // Character physics integration
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ApplyPhysicsToCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdateCharacterPhysics(ACharacter* Character, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ModifyPhysicsForSurvivalState(ACharacter* Character, float HealthPercent, float StaminaPercent, float FearLevel);

    // Collision and impact system
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void HandleCollisionImpact(AActor* Actor, const FVector& ImpactPoint, const FVector& ImpactNormal, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    float CalculateImpactDamage(float ImpactForce) const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool ShouldActivateRagdoll(float ImpactForce) const;

    // Ragdoll system
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void ActivateRagdoll(ACharacter* Character, const FVector& ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void DeactivateRagdoll(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void UpdateRagdollState(ACharacter* Character, float DeltaTime);

    // Destruction system
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void TriggerDestruction(AActor* Actor, const FVector& ImpactPoint, float DestructionForce);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void CreateDestructionFragments(AActor* Actor, const FVector& ImpactPoint, int32 NumFragments);

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    void CleanupDestructionFragments();

    // Physics state queries
    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FCore_PhysicsState GetPhysicsState(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    bool IsActorInRagdoll(ACharacter* Character) const;

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    float GetCurrentGravityScale(AActor* Actor) const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor)
    void ValidatePhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Physics System", CallInEditor)
    void DebugPhysicsState();

    UFUNCTION(BlueprintCallable, Category = "Physics System")
    FString GetPhysicsSystemStatus() const;

protected:
    // Core physics settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    // Physics state tracking
    UPROPERTY(BlueprintReadOnly, Category = "Physics State")
    TMap<AActor*, FCore_PhysicsState> ActorPhysicsStates;

    // Ragdoll management
    UPROPERTY(BlueprintReadOnly, Category = "Ragdoll System")
    TArray<ACharacter*> ActiveRagdolls;

    // Destruction fragments tracking
    UPROPERTY(BlueprintReadOnly, Category = "Destruction System")
    TArray<AActor*> DestructionFragments;

    // Timer handles
    FTimerHandle RagdollUpdateTimer;
    FTimerHandle FragmentCleanupTimer;

private:
    // Internal helper functions
    void InitializePhysicsSettings();
    void SetupTimers();
    void CleanupTimers();
    
    float CalculateSurvivalPhysicsModifier(float HealthPercent, float StaminaPercent, float FearLevel) const;
    void ApplyGravityScale(AActor* Actor, float GravityScale);
    void ApplyFriction(AActor* Actor, float Friction);
    
    bool ValidateActor(AActor* Actor) const;
    void LogPhysicsEvent(const FString& EventName, AActor* Actor, const FString& Details) const;
};