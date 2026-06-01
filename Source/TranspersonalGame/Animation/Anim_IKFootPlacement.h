#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
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
    float JointTargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK")
    bool bIsFootOnGround;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        JointTargetLocation = 0.0f;
        bIsFootOnGround = false;
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
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootForwardOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxIKAdjustment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableDebugDraw;

    FAnim_IKSettings()
    {
        TraceDistance = 150.0f;
        IKInterpSpeed = 15.0f;
        FootHeight = 13.0f;
        FootForwardOffset = 20.0f;
        MaxIKAdjustment = 50.0f;
        bEnableFootIK = true;
        bEnableDebugDraw = false;
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

    // IK Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    // Foot IK Data
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    // Pelvis adjustment
    UPROPERTY(BlueprintReadOnly, Category = "IK Data")
    float PelvisOffset;

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bones")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bones")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Bones")
    FName PelvisBoneName;

    // Main IK update function
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    // Individual foot IK calculation
    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, float DeltaTime);

    // Ground trace for foot placement
    UFUNCTION(BlueprintCallable, Category = "IK")
    bool TraceForGround(FVector StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    // Pelvis adjustment calculation
    UFUNCTION(BlueprintCallable, Category = "IK")
    float CalculatePelvisOffset();

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    FRotator CalculateFootRotationFromNormal(FVector SurfaceNormal);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetDebugDrawEnabled(bool bEnabled);

private:
    // Internal helper functions
    FVector GetFootWorldLocation(FName FootBoneName);
    USkeletalMeshComponent* GetOwnerSkeletalMesh();
    void DrawDebugIK(const FAnim_FootIKData& FootData, FName FootBoneName);
    float InterpolateIKAlpha(float CurrentAlpha, float TargetAlpha, float DeltaTime);
    
    // Cached references
    UPROPERTY()
    USkeletalMeshComponent* OwnerSkeletalMesh;
    
    // Previous frame data for interpolation
    float PreviousPelvisOffset;
    bool bInitialized;
};