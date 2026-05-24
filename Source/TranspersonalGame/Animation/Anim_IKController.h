#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "SharedTypes.h"
#include "Anim_IKController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float DistanceFromGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 13.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHandIK = false;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootHeight = 13.0f;
        MaxIKDistance = 30.0f;
        bEnableFootIK = true;
        bEnableHandIK = false;
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

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Foot IK Functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    // Hand IK Functions
    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void SetLeftHandIKTarget(FVector TargetLocation, FRotator TargetRotation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void SetRightHandIKTarget(FVector TargetLocation, FRotator TargetRotation, float Alpha = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Hand IK")
    void ClearHandIKTargets();

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    FVector GetLeftHandIKLocation() const { return LeftHandIKLocation; }

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    FVector GetRightHandIKLocation() const { return RightHandIKLocation; }

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    FRotator GetLeftHandIKRotation() const { return LeftHandIKRotation; }

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    FRotator GetRightHandIKRotation() const { return RightHandIKRotation; }

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    float GetLeftHandIKAlpha() const { return LeftHandIKAlpha; }

    UFUNCTION(BlueprintPure, Category = "Hand IK")
    float GetRightHandIKAlpha() const { return RightHandIKAlpha; }

    // Settings
    UFUNCTION(BlueprintCallable, Category = "IK Settings")
    void SetIKEnabled(bool bEnabled) { IKSettings.bEnableFootIK = bEnabled; }

    UFUNCTION(BlueprintPure, Category = "IK Settings")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

protected:
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // Foot IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset;

    // Hand IK Data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    FVector LeftHandIKLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    FVector RightHandIKLocation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    FRotator LeftHandIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    FRotator RightHandIKRotation;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    float LeftHandIKAlpha;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hand IK")
    float RightHandIKAlpha;

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftHandBoneName = TEXT("hand_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightHandBoneName = TEXT("hand_r");

private:
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, float DeltaTime);
    FHitResult PerformFootTrace(FVector StartLocation) const;
    float CalculatePelvisOffset() const;
};