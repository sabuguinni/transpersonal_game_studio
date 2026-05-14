#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector FootLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FRotator FootRotation = FRotator::ZeroRotator;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    float GroundDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FVector GroundNormal = FVector::UpVector;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        GroundDistance = 0.0f;
        GroundNormal = FVector::UpVector;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKDistance = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootHeight = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bShowDebugTraces = false;

    FAnim_IKSettings()
    {
        MaxIKDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        TraceDistance = 100.0f;
        FootHeight = 10.0f;
        bEnableIK = true;
        bShowDebugTraces = false;
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

    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIK() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIK() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    float GetHipOffset() const { return HipOffset; }

    // Individual foot tracing
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData TraceFootIK(FName SocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FVector GetFootWorldLocation(FName SocketName) const;

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool PerformFootTrace(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal, float& OutDistance);

    // Settings
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled) { IKSettings.bEnableIK = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool IsIKEnabled() const { return IKSettings.bEnableIK; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings) { IKSettings = NewSettings; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_IKSettings GetIKSettings() const { return IKSettings; }

protected:
    // IK data for each foot
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    // Hip adjustment for overall character positioning
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data", meta = (AllowPrivateAccess = "true"))
    float HipOffset = 0.0f;

    // IK configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration", meta = (AllowPrivateAccess = "true"))
    FAnim_IKSettings IKSettings;

    // Socket names for foot bones
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration", meta = (AllowPrivateAccess = "true"))
    FName LeftFootSocketName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration", meta = (AllowPrivateAccess = "true"))
    FName RightFootSocketName = TEXT("foot_r");

    // Component references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMesh;

    UPROPERTY()
    class UCharacterMovementComponent* CharacterMovement;

private:
    // Internal tracking
    float TargetHipOffset = 0.0f;
    float CurrentHipOffset = 0.0f;
    
    // Helper functions
    float CalculateHipOffset(const FAnim_FootIKData& LeftFoot, const FAnim_FootIKData& RightFoot);
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation);
    bool ShouldPerformIK() const;
};