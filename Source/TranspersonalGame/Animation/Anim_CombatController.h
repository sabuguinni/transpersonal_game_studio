#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_CombatController.generated.h"

UENUM(BlueprintType)
enum class EAnim_CombatState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Attacking   UMETA(DisplayName = "Attacking"),
    Blocking    UMETA(DisplayName = "Blocking"),
    Dodging     UMETA(DisplayName = "Dodging"),
    Stunned     UMETA(DisplayName = "Stunned"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class EAnim_WeaponType : uint8
{
    None        UMETA(DisplayName = "None"),
    Spear       UMETA(DisplayName = "Spear"),
    Club        UMETA(DisplayName = "Club"),
    Stone       UMETA(DisplayName = "Stone"),
    Bow         UMETA(DisplayName = "Bow")
};

USTRUCT(BlueprintType)
struct FAnim_CombatAnimData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    class UAnimMontage* AttackMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    class UAnimMontage* BlockMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    class UAnimMontage* DodgeMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    class UAnimMontage* HitReactionMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    float AttackSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animation")
    float RecoveryTime;

    FAnim_CombatAnimData()
    {
        AttackMontage = nullptr;
        BlockMontage = nullptr;
        DodgeMontage = nullptr;
        HitReactionMontage = nullptr;
        AttackSpeed = 1.0f;
        RecoveryTime = 0.5f;
    }
};

/**
 * Combat Animation Controller - Manages combat-specific animations for prehistoric survival
 * Handles weapon-based attack animations, defensive moves, and combat state transitions
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_CombatController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_CombatController();

protected:
    virtual void BeginPlay() override;

    // Combat State Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State", meta = (AllowPrivateAccess = "true"))
    EAnim_CombatState CurrentCombatState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat State", meta = (AllowPrivateAccess = "true"))
    EAnim_WeaponType CurrentWeaponType;

    // Animation Data per Weapon Type
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Animations")
    TMap<EAnim_WeaponType, FAnim_CombatAnimData> WeaponAnimations;

    // Combat Timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float ComboWindowTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float BlockHoldTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Timing")
    float DodgeCooldown;

    // Component References
    UPROPERTY()
    class USkeletalMeshComponent* OwnerMesh;

    UPROPERTY()
    class UAnimInstance* OwnerAnimInstance;

    // Combat State Tracking
    UPROPERTY()
    float LastAttackTime;

    UPROPERTY()
    float LastDodgeTime;

    UPROPERTY()
    int32 CurrentComboCount;

    UPROPERTY()
    bool bIsInComboWindow;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Combat Action Functions
    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    bool TryStartAttack();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    bool TryStartBlock();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    bool TryStartDodge(FVector DodgeDirection);

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void StopBlock();

    UFUNCTION(BlueprintCallable, Category = "Combat Actions")
    void TriggerHitReaction(float Damage, FVector HitDirection);

    // Weapon Management
    UFUNCTION(BlueprintCallable, Category = "Weapon Management")
    void SetCurrentWeapon(EAnim_WeaponType NewWeaponType);

    UFUNCTION(BlueprintCallable, Category = "Weapon Management")
    void RegisterWeaponAnimations(EAnim_WeaponType WeaponType, const FAnim_CombatAnimData& AnimData);

    // State Queries
    UFUNCTION(BlueprintPure, Category = "Combat State")
    EAnim_CombatState GetCurrentCombatState() const { return CurrentCombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    EAnim_WeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool CanAttack() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool CanBlock() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool CanDodge() const;

    UFUNCTION(BlueprintPure, Category = "Combat State")
    bool IsInCombat() const;

    // Animation Event Handlers
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnBlockMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnDodgeMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnHitReactionMontageEnded(UAnimMontage* Montage, bool bInterrupted);

protected:
    // Internal State Management
    void SetCombatState(EAnim_CombatState NewState);
    void UpdateComboWindow(float DeltaTime);
    void InitializeDefaultAnimations();
    bool PlayMontageForWeapon(EAnim_WeaponType WeaponType, UAnimMontage* FAnim_CombatAnimData::* MontagePtr);
    FAnim_CombatAnimData* GetCurrentWeaponAnimData();

    // Combat Logic
    void ProcessAttackCombo();
    void ResetCombatState();
    float CalculateAttackDamage() const;
    FVector CalculateKnockbackForce(FVector HitDirection) const;

    // Animation Callbacks
    UFUNCTION()
    void OnMontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

    UFUNCTION()
    void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};