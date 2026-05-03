#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "../SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    float EffectorLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FVector JointTarget;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        EffectorLocation = 0.0f;
        JointTarget = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxLegExtension;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName PelvisBoneName;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        FootOffset = 5.0f;
        MaxLegExtension = 25.0f;
        bEnableDebugDraw = false;
        LeftFootBoneName = FName("foot_l");
        RightFootBoneName = FName("foot_r");
        PelvisBoneName = FName("pelvis");
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK data for feet
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Pelvis adjustment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

    // Main IK calculation functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool IsIKEnabled() const { return bIKEnabled; }

    // Individual foot IK calculations
    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    float CalculatePelvisOffset();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector GetFootLocation(FName FootBoneName) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotation(const FVector& HitNormal, const FVector& FootForward) const;

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "IK Events")
    void OnIKUpdated();

    UFUNCTION(BlueprintImplementableEvent, Category = "IK Events")
    void OnGroundHit(const FVector& HitLocation, const FVector& HitNormal);

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    // IK state
    bool bIKEnabled;
    float LastUpdateTime;

    // Previous frame data for interpolation
    FAnim_FootIKData PreviousLeftFootIK;
    FAnim_FootIKData PreviousRightFootIK;
    float PreviousPelvisOffset;

    // Helper functions
    void CacheComponents();
    void InitializeIKData();
    FVector InterpolateFootLocation(const FVector& CurrentLocation, const FVector& TargetLocation, float DeltaTime) const;
    FRotator InterpolateFootRotation(const FRotator& CurrentRotation, const FRotator& TargetRotation, float DeltaTime) const;
    void DrawDebugIK() const;
    bool IsCharacterMoving() const;
    bool ShouldUpdateIK() const;
};