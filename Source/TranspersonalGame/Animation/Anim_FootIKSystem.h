#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "SharedTypes.h"
#include "Anim_FootIKSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector LeftFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector RightFootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator LeftFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator RightFootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float LeftFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float RightFootIKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float HipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsIKActive;

    FAnim_FootIKData()
    {
        LeftFootLocation = FVector::ZeroVector;
        RightFootLocation = FVector::ZeroVector;
        LeftFootRotation = FRotator::ZeroRotator;
        RightFootRotation = FRotator::ZeroRotator;
        LeftFootIKAlpha = 0.0f;
        RightFootIKAlpha = 0.0f;
        HipOffset = 0.0f;
        bIsIKActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxHipOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FName HipBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TEnumAsByte<ECollisionChannel> TraceChannel;

    FAnim_FootIKConfig()
    {
        TraceDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        FootHeight = 5.0f;
        MaxHipOffset = 30.0f;
        LeftFootBoneName = TEXT("foot_l");
        RightFootBoneName = TEXT("foot_r");
        HipBoneName = TEXT("pelvis");
        TraceChannel = ECC_WorldStatic;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootTraceResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Trace")
    bool bHit;

    UPROPERTY(BlueprintReadOnly, Category = "Trace")
    FVector HitLocation;

    UPROPERTY(BlueprintReadOnly, Category = "Trace")
    FVector HitNormal;

    UPROPERTY(BlueprintReadOnly, Category = "Trace")
    float Distance;

    FAnim_FootTraceResult()
    {
        bHit = false;
        HitLocation = FVector::ZeroVector;
        HitNormal = FVector::UpVector;
        Distance = 0.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_FootIKSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_FootIKSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // IK System Control
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void EnableFootIK(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    bool IsFootIKEnabled() const { return IKData.bIsIKActive; }

    // Foot Tracing
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    FAnim_FootTraceResult TraceForGround(const FVector& StartLocation, const FVector& EndLocation);

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void UpdateFootTraces();

    // IK Data Access
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FAnim_FootIKData GetFootIKData() const { return IKData; }

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetFootIKData(const FAnim_FootIKData& NewData);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetFootIKConfig(const FAnim_FootIKConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FAnim_FootIKConfig GetFootIKConfig() const { return Config; }

    // Individual Foot Control
    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetLeftFootIK(const FVector& Location, const FRotator& Rotation, float Alpha);

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetRightFootIK(const FVector& Location, const FRotator& Rotation, float Alpha);

    UFUNCTION(BlueprintCallable, Category = "Animation|FootIK")
    void SetHipOffset(float Offset);

    // Utility Functions
    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FVector GetFootWorldLocation(bool bLeftFoot) const;

    UFUNCTION(BlueprintPure, Category = "Animation|FootIK")
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData IKData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAnim_FootIKConfig Config;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    FAnim_FootTraceResult LeftFootTrace;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    FAnim_FootTraceResult RightFootTrace;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    USkeletalMeshComponent* SkeletalMeshComponent;

private:
    void InitializeComponent();
    void UpdateSingleFootIK(bool bLeftFoot, float DeltaTime);
    void CalculateHipOffset();
    FVector GetBoneWorldLocation(const FName& BoneName) const;
    void InterpolateIKValues(float DeltaTime);
    bool ValidateConfiguration() const;
    
    // Cached values for interpolation
    FVector TargetLeftFootLocation;
    FVector TargetRightFootLocation;
    FRotator TargetLeftFootRotation;
    FRotator TargetRightFootRotation;
    float TargetHipOffset;
};