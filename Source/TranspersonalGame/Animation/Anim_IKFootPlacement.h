#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "SharedTypes.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName FootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    FName IKBoneName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    FVector FootLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    float IKAlpha;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootBoneName = NAME_None;
        IKBoneName = NAME_None;
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKFootPlacement : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKFootPlacement();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetupFootIK(const FName& LeftFootBone, const FName& RightFootBone, const FName& LeftIKBone, const FName& RightIKBone);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsFootIKEnabled() const { return bFootIKEnabled; }

    // IK Data Access
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetCapsuleHalfHeight() const { return CapsuleHalfHeight; }

    // Manual IK Update
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK();

    // Ground Detection
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool TraceForGround(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal, float& OutDistance);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    bool bFootIKEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float InterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    float MaxIKDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Config")
    TArray<TEnumAsByte<EObjectTypeQuery>> TraceObjectTypes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    float HipOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK Data")
    float CapsuleHalfHeight;

private:
    class ACharacter* OwnerCharacter;
    class USkeletalMeshComponent* SkeletalMeshComp;
    class UCapsuleComponent* CapsuleComp;

    void UpdateFootIKData(FAnim_FootIKData& FootData, float DeltaTime);
    FVector GetFootWorldLocation(const FName& BoneName) const;
    void CalculateHipOffset();
};