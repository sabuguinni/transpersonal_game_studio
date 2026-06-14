#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/DataAsset.h"
#include "SharedTypes.h"
#include "Anim_BlendSpaceManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UBlendSpace> LocomotionBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSoftObjectPtr<UBlendSpace1D> SpeedBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float MaxRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float BlendSpaceScale;

    FAnim_BlendSpaceConfig()
    {
        MaxWalkSpeed = 300.0f;
        MaxRunSpeed = 600.0f;
        BlendSpaceScale = 1.0f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    FAnim_BlendSpaceConfig BlendSpaceConfig;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float CurrentSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float CurrentDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsMoving;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsRunning;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateBlendSpaceValues(const FVector& Velocity, float MaxSpeed);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceConfig(const FAnim_BlendSpaceConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetNormalizedSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    FVector2D GetBlendSpaceInput() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void LoadBlendSpaces();

private:
    void CalculateMovementValues(const FVector& Velocity);
    void UpdateBlendSpaceParameters();

    UPROPERTY()
    class ACharacter* OwnerCharacter;

    float PreviousSpeed;
    float SpeedChangeRate;
};