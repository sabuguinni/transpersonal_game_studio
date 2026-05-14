#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "TranspersonalGame.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    float DistanceFromGround;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK")
    FVector GroundNormal;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
        GroundNormal = FVector::UpVector;
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
    float FootOffset = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float MaxFootAngle = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bDrawDebugTraces = false;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        InterpSpeed = 15.0f;
        FootOffset = 5.0f;
        bEnableFootRotation = true;
        MaxFootAngle = 45.0f;
        bDrawDebugTraces = false;
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
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    bool TraceForGround(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FRotator CalculateFootRotation(const FVector& GroundNormal, const FRotator& CurrentRotation);

    // Getters for animation blueprint
    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIKData; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    float GetPelvisOffset() const { return PelvisOffset; }

    UFUNCTION(BlueprintPure, Category = "Foot IK")
    bool IsIKEnabled() const { return bEnableIK; }

    // Configuration functions
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKEnabled(bool bEnabled) { bEnableIK = bEnabled; }

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void SetIKSettings(const FAnim_IKSettings& NewSettings) { IKSettings = NewSettings; }

protected:
    // IK data for each foot
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData LeftFootIKData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    FAnim_FootIKData RightFootIKData;

    // Pelvis adjustment
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    float PelvisOffset;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Foot IK")
    float TargetPelvisOffset;

    // Component references
    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    UPROPERTY()
    ACharacter* OwnerCharacter;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    FAnim_IKSettings IKSettings;

    // Socket names for feet
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    FName LeftFootSocketName = "foot_l";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    FName RightFootSocketName = "foot_r";

    // Control flags
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    bool bEnableIK = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    bool bEnablePelvisAdjustment = true;

    // Collision settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    TEnumAsByte<ECollisionChannel> TraceChannel = ECC_Visibility;

    // Interpolation speeds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    float PelvisInterpSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Configuration")
    float FootInterpSpeed = 15.0f;

private:
    // Helper functions
    void UpdatePelvisOffset(float DeltaTime);
    float CalculateOptimalPelvisOffset();
    void InterpolateFootIKData(FAnim_FootIKData& Current, const FAnim_FootIKData& Target, float DeltaTime);
    bool IsCharacterMoving() const;
    void DrawDebugInfo() const;
};