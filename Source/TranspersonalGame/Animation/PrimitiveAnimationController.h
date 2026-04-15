#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "PrimitiveAnimationController.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

UENUM(BlueprintType)
enum class EAnim_MovementState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Walking     UMETA(DisplayName = "Walking"),  
    Running     UMETA(DisplayName = "Running"),
    Sprinting   UMETA(DisplayName = "Sprinting"),
    Jumping     UMETA(DisplayName = "Jumping"),
    Falling     UMETA(DisplayName = "Falling"),
    Landing     UMETA(DisplayName = "Landing"),
    Crouching   UMETA(DisplayName = "Crouching"),
    Crawling    UMETA(DisplayName = "Crawling")
};

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Neutral         UMETA(DisplayName = "Neutral"),
    Alert           UMETA(DisplayName = "Alert"),
    Attacking       UMETA(DisplayName = "Attacking"),
    Blocking        UMETA(DisplayName = "Blocking"),
    Dodging         UMETA(DisplayName = "Dodging"),
    Stunned         UMETA(DisplayName = "Stunned"),
    Dead            UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_MovementData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Speed = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float Direction = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsMoving = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsInAir = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    bool bIsCrouching = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    EAnim_MovementState MovementState = EAnim_MovementState::Idle;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_CombatData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    EAnim_CombatState CombatState = EAnim_CombatState::Neutral;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsAttacking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    bool bIsBlocking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    int32 ComboIndex = 0;
};

/**
 * Primitive Animation Controller - Handles basic character animations for prehistoric survival
 * Manages movement states, combat animations, and environmental adaptations
 * Designed for realistic human primitive movement in dangerous prehistoric world
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPrimitiveAnimationController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPrimitiveAnimationController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CORE ANIMATION CONTROL ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateMovementState(float Speed, FVector Velocity, bool bIsInAir, bool bIsCrouching);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCombatState(EAnim_CombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayAttackAnimation(int32 AttackIndex = 0);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayDodgeAnimation(FVector DodgeDirection);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayJumpAnimation();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayLandingAnimation(float FallHeight);

    // === ENVIRONMENTAL ADAPTATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void AdaptToTerrain(float SlopeAngle, ESurfaceType SurfaceType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetCarryingState(bool bIsCarrying, float CarryWeight = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetFatigueLevel(float FatiguePercent);

    // === SURVIVAL ANIMATIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayCraftingAnimation(ECraftingType CraftingType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGatheringAnimation(EResourceType ResourceType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayRestAnimation(bool bIsSleeping = false);

    // === GETTERS ===
    
    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_MovementData GetMovementData() const { return MovementData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    FAnim_CombatData GetCombatData() const { return CombatData; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    EAnim_MovementState GetCurrentMovementState() const { return MovementData.MovementState; }

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsPlayingMontage() const;

protected:
    // === CORE DATA ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    FAnim_MovementData MovementData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Animation Data")
    FAnim_CombatData CombatData;

    // === COMPONENT REFERENCES ===
    
    UPROPERTY()
    TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY()
    TWeakObjectPtr<UAnimInstance> AnimInstance;

    // === ANIMATION ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> DodgeMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> JumpMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> CraftingMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UAnimMontage> GatheringMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Assets")
    TObjectPtr<UBlendSpace> MovementBlendSpace;

    // === ANIMATION PARAMETERS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float WalkThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float RunThreshold = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float SprintThreshold = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float AnimationBlendSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Settings")
    float TerrainAdaptationSpeed = 3.0f;

    // === ENVIRONMENTAL STATE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    float CurrentSlopeAngle = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    ESurfaceType CurrentSurfaceType = ESurfaceType::Grass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    bool bIsCarrying = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    float CarryWeight = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Environmental")
    float FatigueLevel = 0.0f;

private:
    // === INTERNAL METHODS ===
    
    void InitializeComponents();
    void UpdateMovementBlending(float DeltaTime);
    void UpdateCombatBlending(float DeltaTime);
    void HandleStateTransitions();
    EAnim_MovementState CalculateMovementState(float Speed, bool bIsInAir, bool bIsCrouching);
    void PlayMontageIfValid(UAnimMontage* Montage, float PlayRate = 1.0f);

    // === CACHED VALUES ===
    
    float LastUpdateTime = 0.0f;
    FVector LastVelocity = FVector::ZeroVector;
    bool bWasInAir = false;
};