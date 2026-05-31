#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator TargetRotation = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector SurfaceNormal = FVector::UpVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    bool bIsGrounded = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float LerpSpeed = 15.0f;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_IKSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "500.0"))
    float TraceDistance = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float FootHeight = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "50.0"))
    float InterpSpeed = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MaxIKAdjustment = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinGroundAngle = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableHipAdjustment = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HipAdjustmentAlpha = 0.5f;
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

    // Main IK functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void PerformFootTrace(FAnim_FootData& FootData, const FVector& FootWorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void CalculateHipOffset();

    // Getters for animation blueprint
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foot IK")
    FAnim_FootData GetLeftFootData() const { return LeftFootData; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foot IK")
    FAnim_FootData GetRightFootData() const { return RightFootData; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foot IK")
    float GetHipOffset() const { return CurrentHipOffset; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const { return bEnableIK; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void ResetIK();

protected:
    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "References")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    USkeletalMeshComponent* CharacterMesh;

    // Foot data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootData LeftFootData;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootData RightFootData;

    // Hip adjustment
    UPROPERTY(BlueprintReadOnly, Category = "Hip IK")
    float CurrentHipOffset = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Hip IK")
    float TargetHipOffset = 0.0f;

    // Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FAnim_IKSettings IKSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDebugDraw = false;

    // Bone names
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName = TEXT("foot_l");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName = TEXT("foot_r");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName HipBoneName = TEXT("pelvis");

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_WorldStatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision")
    TArray<TEnumAsByte<EObjectType>> ObjectTypesToTrace;

private:
    // Internal state
    bool bIsInitialized = false;
    float LastUpdateTime = 0.0f;
    
    // Helper functions
    void InitializeComponent();
    FVector GetFootWorldLocation(const FName& BoneName) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
    void UpdateFootData(FAnim_FootData& FootData, float DeltaTime);
    void DrawDebugInfo() const;
    bool ShouldUpdateIK() const;
};