#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_RealTimeIKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKBoneTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsActive;

    FAnim_IKBoneTarget()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        BlendWeight = 1.0f;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName FootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName IKBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootBoneName = NAME_None;
        IKBoneName = NAME_None;
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_LookAtIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FName HeadBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float MaxLookAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    bool bIsActive;

    FAnim_LookAtIKData()
    {
        HeadBoneName = TEXT("head");
        LookAtTarget = FVector::ZeroVector;
        LookAtAlpha = 0.0f;
        MaxLookAngle = 90.0f;
        bIsActive = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAnim_OnIKTargetReached, FName, BoneName, FVector, TargetLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAnim_OnFootGroundContact, FName, FootBone);

/**
 * Real-time IK system for procedural animation adjustments
 * Handles foot IK, look-at IK, and general bone targeting
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_RealTimeIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_RealTimeIKSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === IK TARGET MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void AddIKTarget(FName BoneName, FVector TargetLocation, FRotator TargetRotation, float BlendWeight = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void RemoveIKTarget(FName BoneName);
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void UpdateIKTarget(FName BoneName, FVector NewTargetLocation, FRotator NewTargetRotation);
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetIKTargetBlendWeight(FName BoneName, float BlendWeight);

    // === FOOT IK SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableFootIK(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetFootIKSettings(float TraceDistance, float InterpSpeed, float OffsetThreshold);
    
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetFootIKData(FName FootBone) const;
    
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    // === LOOK AT IK SYSTEM ===
    
    UFUNCTION(BlueprintCallable, Category = "Look At IK")
    void SetLookAtTarget(FVector TargetLocation, float BlendWeight = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Look At IK")
    void ClearLookAtTarget();
    
    UFUNCTION(BlueprintCallable, Category = "Look At IK")
    void SetLookAtSettings(float MaxAngle, float InterpSpeed);

    // === UTILITY FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void SetIKSystemEnabled(bool bEnable);
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    bool IsIKSystemEnabled() const { return bIKSystemEnabled; }
    
    UFUNCTION(BlueprintCallable, Category = "IK System")
    void ResetAllIKTargets();

    // === EVENTS ===
    
    UPROPERTY(BlueprintAssignable, Category = "IK Events")
    FAnim_OnIKTargetReached OnIKTargetReached;
    
    UPROPERTY(BlueprintAssignable, Category = "IK Events")
    FAnim_OnFootGroundContact OnFootGroundContact;

protected:
    // === CORE COMPONENTS ===
    
    UPROPERTY()
    class ACharacter* OwnerCharacter;
    
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComp;
    
    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // === IK SYSTEM SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bIKSystemEnabled;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float IKUpdateFrequency;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float IKBlendSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    float IKTargetReachedThreshold;

    // === FOOT IK SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bFootIKEnabled;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootTraceDistance;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootOffsetThreshold;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    TArray<FName> FootBoneNames;

    // === LOOK AT IK SETTINGS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    bool bLookAtIKEnabled;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FName HeadBoneName;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtInterpSpeed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float MaxLookAtAngle;

    // === RUNTIME DATA ===
    
    UPROPERTY()
    TArray<FAnim_IKBoneTarget> ActiveIKTargets;
    
    UPROPERTY()
    TArray<FAnim_FootIKData> FootIKData;
    
    UPROPERTY()
    FAnim_LookAtIKData LookAtData;
    
    UPROPERTY()
    float LastIKUpdateTime;

    // === INTERNAL FUNCTIONS ===
    
    void InitializeIKSystem();
    void UpdateIKTargets(float DeltaTime);
    void UpdateLookAtIK(float DeltaTime);
    void ProcessIKTarget(FAnim_IKBoneTarget& IKTarget, float DeltaTime);
    
    FVector PerformFootTrace(FName FootBone, FVector StartLocation);
    void CalculateFootIKOffset(FAnim_FootIKData& FootData, FVector GroundLocation);
    
    bool IsValidBone(FName BoneName) const;
    FVector GetBoneWorldLocation(FName BoneName) const;
    FRotator GetBoneWorldRotation(FName BoneName) const;
    
    void ApplyIKToAnimInstance();
    void DebugDrawIKTargets();
};