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
struct TRANSPERSONALGAME_API FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FVector FootLocation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    FRotator FootRotation;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float IKAlpha;

    UPROPERTY(BlueprintReadOnly, Category = "IK")
    float FootOffset;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        FootOffset = 0.0f;
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

    // IK foot placement functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetLeftFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData GetRightFootIKData() const;

    UFUNCTION(BlueprintCallable, Category = "IK")
    float GetPelvisOffset() const;

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "IK")
    bool IsIKEnabled() const;

protected:
    // Cached references
    UPROPERTY()
    ACharacter* OwnerCharacter;

    UPROPERTY()
    USkeletalMeshComponent* SkeletalMeshComponent;

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float FootHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName RightFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "IK Settings")
    FName PelvisBoneName;

    // IK data
    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
    float PelvisOffset;

private:
    // Internal state
    float TargetPelvisOffset;
    
    // Helper functions
    FAnim_FootIKData CalculateFootIK(const FName& FootBoneName, const FAnim_FootIKData& CurrentData, float DeltaTime);
    FVector GetFootWorldLocation(const FName& FootBoneName) const;
    bool TraceFootPlacement(const FVector& StartLocation, FVector& OutHitLocation, FVector& OutHitNormal) const;
    FRotator CalculateFootRotationFromNormal(const FVector& SurfaceNormal) const;
    void UpdatePelvisOffset(float DeltaTime);
};

#include "Anim_IKFootPlacement.generated.h"