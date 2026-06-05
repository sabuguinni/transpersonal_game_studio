#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSequence.h"
#include "SharedTypes.h"
#include "Anim_BlendSpaceController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* MovementBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace1D* SpeedBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float CurrentSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    FVector2D MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsInAir;

    FAnim_BlendSpaceData()
    {
        MovementBlendSpace = nullptr;
        SpeedBlendSpace = nullptr;
        CurrentSpeed = 0.0f;
        MovementDirection = FVector2D::ZeroVector;
        bIsInAir = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxWalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MaxRunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BlendSpaceSmoothing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float DirectionChangeThreshold;

    FAnim_BlendSpaceConfig()
    {
        MaxWalkSpeed = 400.0f;
        MaxRunSpeed = 800.0f;
        BlendSpaceSmoothing = 5.0f;
        DirectionChangeThreshold = 0.1f;
    }
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnim_BlendSpaceController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Blend Space Management
    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void InitializeBlendSpaces();

    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void UpdateBlendSpaceValues(float Speed, const FVector2D& Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void SetMovementBlendSpace(UBlendSpace* NewBlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void SetSpeedBlendSpace(UBlendSpace1D* NewBlendSpace);

    // Blend Space Queries
    UFUNCTION(BlueprintPure, Category = "Animation|BlendSpace")
    FAnim_BlendSpaceData GetCurrentBlendSpaceData() const { return BlendSpaceData; }

    UFUNCTION(BlueprintPure, Category = "Animation|BlendSpace")
    float GetNormalizedSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Animation|BlendSpace")
    FVector2D GetSmoothedDirection() const { return SmoothedDirection; }

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void SetBlendSpaceConfig(const FAnim_BlendSpaceConfig& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Animation|BlendSpace")
    FAnim_BlendSpaceConfig GetBlendSpaceConfig() const { return Config; }

    // Animation Sampling
    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    UAnimSequence* SampleBlendSpaceAnimation(float Speed, const FVector2D& Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation|BlendSpace")
    void CreateDefaultBlendSpaces();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Space")
    FAnim_BlendSpaceData BlendSpaceData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FAnim_BlendSpaceConfig Config;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    FVector2D SmoothedDirection;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float SmoothedSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime")
    float LastUpdateTime;

private:
    void UpdateSmoothedValues(float DeltaTime, float TargetSpeed, const FVector2D& TargetDirection);
    void ValidateBlendSpaces();
    UBlendSpace* CreateMovementBlendSpace();
    UBlendSpace1D* CreateSpeedBlendSpace();
};