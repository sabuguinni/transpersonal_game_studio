#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/World.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadWrite, Category = "IK")
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
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bEnableFootRotation;

    FAnim_IKSettings()
    {
        TraceDistance = 50.0f;
        IKInterpSpeed = 15.0f;
        FootOffset = 5.0f;
        bEnableFootIK = true;
        bEnableFootRotation = true;
    }
};

UCLASS(Blueprintable, BlueprintType, ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
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

    // Bone names for IK
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bone Names")
    FName PelvisBoneName;

    // Character reference
    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Character")
    class USkeletalMeshComponent* CharacterMesh;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const { return LeftFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const { return RightFootIK; }

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    void ResetIK();

private:
    // Internal IK calculation functions
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, float DeltaTime);
    FVector PerformFootTrace(const FVector& FootLocation, float& OutDistance);
    FRotator CalculateFootRotation(const FVector& ImpactLocation, const FVector& ImpactNormal);
    float CalculateIKAlpha(float DistanceFromGround);
    
    // Interpolation helpers
    FVector InterpFootLocation(const FVector& Current, const FVector& Target, float DeltaTime);
    FRotator InterpFootRotation(const FRotator& Current, const FRotator& Target, float DeltaTime);
    
    // Cached values for smooth interpolation
    FVector LastLeftFootLocation;
    FVector LastRightFootLocation;
    FRotator LastLeftFootRotation;
    FRotator LastRightFootRotation;
    
    // Pelvis adjustment
    float PelvisOffset;
    float LastPelvisOffset;
};