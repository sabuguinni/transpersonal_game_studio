#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Anim_IKController.generated.h"

UENUM(BlueprintType)
enum class EAnim_IKBoneType : uint8
{
    LeftFoot        UMETA(DisplayName = "Left Foot"),
    RightFoot       UMETA(DisplayName = "Right Foot"),
    LeftHand        UMETA(DisplayName = "Left Hand"),
    RightHand       UMETA(DisplayName = "Right Hand"),
    Head            UMETA(DisplayName = "Head"),
    Spine           UMETA(DisplayName = "Spine")
};

USTRUCT(BlueprintType)
struct FAnim_IKBoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bone")
    float BlendSpeed;

    FAnim_IKBoneData()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        IKWeight = 0.0f;
        bIsActive = false;
        BlendSpeed = 5.0f;
    }
};

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float GroundDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKWeight = 0.0f;
        GroundDistance = 0.0f;
        bIsGrounded = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    TMap<EAnim_IKBoneType, FAnim_IKBoneData> IKBones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKTraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootIKCapsuleHalfHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableLookAtIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At")
    float LookAtWeight;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void SetIKTarget(EAnim_IKBoneType BoneType, FVector TargetLocation, FRotator TargetRotation, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "IK Control")
    void ActivateIK(EAnim_IKBoneType BoneType, bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Look At")
    void SetLookAtTarget(FVector Target, float Weight = 1.0f);

    UFUNCTION(BlueprintPure, Category = "IK Control")
    FAnim_IKBoneData GetIKBoneData(EAnim_IKBoneType BoneType) const;

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Look At")
    FVector GetLookAtTarget() const { return LookAtTarget; }

    UFUNCTION(BlueprintPure, Category = "Look At")
    float GetLookAtWeight() const { return LookAtWeight; }

private:
    void InitializeIKBones();
    void UpdateIKWeights(float DeltaTime);
    bool TraceForGround(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);
    FRotator CalculateFootRotationFromNormal(FVector GroundNormal);
    
    UPROPERTY()
    USkeletalMeshComponent* OwnerMeshComponent;

    UPROPERTY()
    float LastUpdateTime;
};