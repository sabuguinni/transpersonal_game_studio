#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Anim_FootIKSystem.generated.h"

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

    FAnim_FootIKData()
    {
        FootLocation = FVector::ZeroVector;
        FootRotation = FRotator::ZeroRotator;
        IKAlpha = 0.0f;
        DistanceFromGround = 0.0f;
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

    // Core IK calculation
    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    FAnim_FootIKData CalculateFootIK(const FName& SocketName, const FVector& RootLocation);

    UFUNCTION(BlueprintCallable, Category = "Foot IK")
    void UpdateFootIK(float DeltaTime);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float TraceDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float IKInterpSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    float MaxFootOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bEnableFootIK;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foot IK Settings")
    bool bShowDebugTraces;

    // Foot data
    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData LeftFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    FAnim_FootIKData RightFootIK;

    UPROPERTY(BlueprintReadOnly, Category = "Foot IK Data")
    float HipOffset;

private:
    UPROPERTY()
    USkeletalMeshComponent* OwnerMesh;

    FVector LastRootLocation;
    float CurrentHipOffset;

    // Helper functions
    FHitResult PerformFootTrace(const FVector& StartLocation, const FVector& EndLocation);
    FRotator CalculateFootRotationFromNormal(const FVector& Normal);
    void UpdateHipOffset(float DeltaTime);
};