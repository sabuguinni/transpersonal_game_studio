#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Anim_IKController.generated.h"

USTRUCT(BlueprintType)
struct FAnim_IKFootData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float KneeTargetOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    bool bIsGrounded = false;

    FAnim_IKFootData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        KneeTargetOffset = 0.0f;
        bIsGrounded = false;
    }
};

USTRUCT(BlueprintType)
struct FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHandIK = false;

    FAnim_IKSettings()
    {
        TraceDistance = 100.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        MaxIKAdjustment = 50.0f;
        bEnableFootIK = true;
        bEnableHandIK = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_IKController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_IKController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_IKFootData CalculateFootIK(const FName& SocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FVector PerformFootTrace(const FVector& StartLocation, float& OutDistance);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotation(const FVector& ImpactNormal);

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_IKFootData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_IKFootData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintPure, Category = "IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintPure, Category = "IK")
    bool IsIKEnabled() const { return IKSettings.bEnableFootIK; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

protected:
    // IK data
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_IKFootData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset = 0.0f;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    FAnim_IKSettings IKSettings;

    // Component references
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    class USkeletalMeshComponent* MeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    class UCapsuleComponent* CapsuleComponent;

    // Socket names for foot bones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    FName LeftFootSocketName = "foot_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (AllowPrivateAccess = "true"))
    FName RightFootSocketName = "foot_r";

private:
    void CacheComponents();
    float CalculatePelvisOffset();
    float InterpolateIKValue(float CurrentValue, float TargetValue, float InterpSpeed, float DeltaTime);
    bool ShouldUpdateIK();
};