#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimSequence.h"
#include "Anim_BlendSpaceManager.generated.h"

UENUM(BlueprintType)
enum class EAnim_BlendSpaceType : uint8
{
    Locomotion      UMETA(DisplayName = "Locomotion"),
    Directional     UMETA(DisplayName = "Directional"),
    Lean            UMETA(DisplayName = "Lean"),
    Aim             UMETA(DisplayName = "Aim"),
    Custom          UMETA(DisplayName = "Custom")
};

USTRUCT(BlueprintType)
struct FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BlendSpace")
    FString BlendSpaceName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BlendSpace")
    EAnim_BlendSpaceType BlendSpaceType;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BlendSpace")
    UBlendSpace* BlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "BlendSpace")
    UBlendSpace1D* BlendSpace1D;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    FVector2D InputRange;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    FVector2D CurrentInput;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Parameters")
    float BlendTime;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
    TArray<UAnimSequence*> SourceAnimations;

    FAnim_BlendSpaceData()
        : BlendSpaceName(TEXT("Default"))
        , BlendSpaceType(EAnim_BlendSpaceType::Locomotion)
        , BlendSpace(nullptr)
        , BlendSpace1D(nullptr)
        , InputRange(FVector2D(-1.0f, 1.0f))
        , CurrentInput(FVector2D::ZeroVector)
        , BlendTime(0.2f)
    {
    }
};

USTRUCT(BlueprintType)
struct FAnim_LocomotionBlendSpace
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
    UBlendSpace* WalkBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
    UBlendSpace* RunBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
    UBlendSpace* CrouchBlendSpace;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
    UBlendSpace1D* SpeedBlendSpace;

    // Animation sequences for blend space creation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkForward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkBackward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkLeft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkRight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkForwardLeft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkForwardRight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkBackwardLeft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Walk Animations")
    UAnimSequence* WalkBackwardRight;

    // Run animations
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Animations")
    UAnimSequence* RunForward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Animations")
    UAnimSequence* RunBackward;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Animations")
    UAnimSequence* RunLeft;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Run Animations")
    UAnimSequence* RunRight;

    FAnim_LocomotionBlendSpace()
        : WalkBlendSpace(nullptr)
        , RunBlendSpace(nullptr)
        , CrouchBlendSpace(nullptr)
        , SpeedBlendSpace(nullptr)
        , WalkForward(nullptr)
        , WalkBackward(nullptr)
        , WalkLeft(nullptr)
        , WalkRight(nullptr)
        , WalkForwardLeft(nullptr)
        , WalkForwardRight(nullptr)
        , WalkBackwardLeft(nullptr)
        , WalkBackwardRight(nullptr)
        , RunForward(nullptr)
        , RunBackward(nullptr)
        , RunLeft(nullptr)
        , RunRight(nullptr)
    {
    }
};

/**
 * Manages blend spaces for character animation
 * Creates and configures locomotion, directional, and custom blend spaces
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAnim_BlendSpaceManager : public UDataAsset
{
    GENERATED_BODY()

public:
    UAnim_BlendSpaceManager();

    // Locomotion blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Locomotion")
    FAnim_LocomotionBlendSpace LocomotionData;

    // Custom blend spaces
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Custom")
    TArray<FAnim_BlendSpaceData> CustomBlendSpaces;

    // Blend space creation and management
    UFUNCTION(BlueprintCallable, Category = "BlendSpace Management")
    UBlendSpace* CreateLocomotionBlendSpace(const FString& BlendSpaceName, 
                                           const TArray<UAnimSequence*>& Animations);

    UFUNCTION(BlueprintCallable, Category = "BlendSpace Management")
    UBlendSpace1D* CreateSpeedBlendSpace(const FString& BlendSpaceName,
                                        UAnimSequence* SlowAnim,
                                        UAnimSequence* FastAnim);

    UFUNCTION(BlueprintCallable, Category = "BlendSpace Management")
    void SetupWalkBlendSpace();

    UFUNCTION(BlueprintCallable, Category = "BlendSpace Management")
    void SetupRunBlendSpace();

    UFUNCTION(BlueprintCallable, Category = "BlendSpace Management")
    void SetupCrouchBlendSpace();

    // Blend space input calculation
    UFUNCTION(BlueprintPure, Category = "Input Calculation")
    FVector2D CalculateLocomotionInput(float Speed, float Direction) const;

    UFUNCTION(BlueprintPure, Category = "Input Calculation")
    float CalculateSpeedInput(float CurrentSpeed, float MaxSpeed) const;

    UFUNCTION(BlueprintPure, Category = "Input Calculation")
    FVector2D CalculateDirectionalInput(const FVector& Velocity, const FVector& ForwardVector) const;

    // Blend space validation
    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBlendSpace(UBlendSpace* BlendSpace) const;

    UFUNCTION(BlueprintCallable, Category = "Validation")
    bool ValidateBlendSpace1D(UBlendSpace1D* BlendSpace1D) const;

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Utilities")
    void RefreshAllBlendSpaces();

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    TArray<UBlendSpace*> GetAllBlendSpaces() const;

    UFUNCTION(BlueprintCallable, Category = "Utilities")
    UBlendSpace* GetBlendSpaceByType(EAnim_BlendSpaceType BlendSpaceType) const;

protected:
    // Internal blend space configuration
    void ConfigureBlendSpaceAxes(UBlendSpace* BlendSpace, 
                                const FString& XAxisName = TEXT("Speed"),
                                const FString& YAxisName = TEXT("Direction")) const;

    void AddAnimationToBlendSpace(UBlendSpace* BlendSpace, 
                                 UAnimSequence* Animation,
                                 const FVector& Position) const;

    void AddAnimationToBlendSpace1D(UBlendSpace1D* BlendSpace1D,
                                   UAnimSequence* Animation,
                                   float Position) const;

private:
    // Default blend space parameters
    static constexpr float DEFAULT_BLEND_TIME = 0.2f;
    static constexpr float MAX_LOCOMOTION_SPEED = 600.0f;
    static constexpr float MAX_DIRECTION_ANGLE = 180.0f;
};