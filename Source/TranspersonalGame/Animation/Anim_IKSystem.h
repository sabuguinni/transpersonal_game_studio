#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Anim_IKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKBone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName BoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator TargetRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float Weight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bEnabled;

    FAnim_IKBone()
    {
        BoneName = NAME_None;
        TargetLocation = FVector::ZeroVector;
        TargetRotation = FRotator::ZeroRotator;
        Weight = 1.0f;
        bEnabled = true;
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
    float IKWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bHitGround;

    FAnim_FootIKData()
    {
        FootBoneName = NAME_None;
        IKBoneName = NAME_None;
        FootOffset = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKWeight = 1.0f;
        TraceDistance = 50.0f;
        bHitGround = false;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    bool bEnableLookAtIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float FootIKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FAnim_IKBone LeftHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hand IK")
    FAnim_IKBone RightHandIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FAnim_IKBone HeadIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    FVector LookAtTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Look At IK")
    float LookAtInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK System")
    TEnumAsByte<ECollisionChannel> GroundTraceChannel;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, const FName& IKBoneName, float TraceDistance);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void SetHandIKTarget(bool bLeftHand, const FVector& TargetLocation, const FRotator& TargetRotation, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Look At IK")
    void SetLookAtTarget(const FVector& Target, float Weight);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    void EnableIKSystem(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FVector GetBoneWorldLocation(const FName& BoneName) const;

    UFUNCTION(BlueprintCallable, Category = "IK System")
    FRotator GetBoneWorldRotation(const FName& BoneName) const;

private:
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    void InitializeComponent();
    FHitResult PerformGroundTrace(const FVector& StartLocation, float TraceDistance);
    FVector CalculateFootPlacement(const FHitResult& HitResult, const FVector& FootLocation);
    FRotator CalculateFootRotation(const FHitResult& HitResult);
    void UpdateLookAtIK(float DeltaTime);
    void UpdateHandIK(float DeltaTime);
    
    // Cached bone indices for performance
    int32 LeftFootBoneIndex;
    int32 RightFootBoneIndex;
    int32 LeftHandBoneIndex;
    int32 RightHandBoneIndex;
    int32 HeadBoneIndex;
    
    void CacheBoneIndices();
};