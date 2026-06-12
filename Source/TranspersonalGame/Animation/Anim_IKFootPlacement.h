#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Anim_IKFootPlacement.generated.h"

USTRUCT(BlueprintType)
struct FAnim_FootIKData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FootLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator FootRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IKAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsGrounded;

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        bIsGrounded = false;
    }
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

    // IK settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    float FootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName LeftFootBoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    FName RightFootBoneName;

    // Foot IK data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "IK Data")
    float HipOffset;

    // IK functions
    UFUNCTION(BlueprintCallable, Category = "IK")
    void UpdateFootIK();

    UFUNCTION(BlueprintCallable, Category = "IK")
    FAnim_FootIKData CalculateFootIK(FName FootBoneName, const FVector& FootLocation);

    UFUNCTION(BlueprintCallable, Category = "IK")
    void SetIKEnabled(bool bEnabled);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "IK Settings")
    bool bIKEnabled;

private:
    FVector TraceForGround(const FVector& StartLocation);
    FRotator CalculateFootRotation(const FVector& FootLocation, const FVector& GroundNormal);
    
    UPROPERTY()
    class ACharacter* OwningCharacter;
    
    UPROPERTY()
    class USkeletalMeshComponent* SkeletalMeshComponent;
};