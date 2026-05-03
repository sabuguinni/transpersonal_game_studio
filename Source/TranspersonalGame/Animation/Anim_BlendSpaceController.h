#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../SharedTypes.h"
#include "Anim_BlendSpaceController.generated.h"

UENUM(BlueprintType)
enum class EAnim_BlendSpaceType : uint8
{
    Locomotion      UMETA(DisplayName = "Locomotion"),
    Combat          UMETA(DisplayName = "Combat"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Injured         UMETA(DisplayName = "Injured"),
    Exhausted       UMETA(DisplayName = "Exhausted"),
    Afraid          UMETA(DisplayName = "Afraid")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    EAnim_BlendSpaceType BlendSpaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace* BlendSpaceAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    class UBlendSpace1D* BlendSpace1DAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float XAxisValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float YAxisValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsActive;

    FAnim_BlendSpaceData()
    {
        BlendSpaceType = EAnim_BlendSpaceType::Locomotion;
        BlendSpaceAsset = nullptr;
        BlendSpace1DAsset = nullptr;
        XAxisValue = 0.0f;
        YAxisValue = 0.0f;
        BlendWeight = 1.0f;
        bIsActive = false;
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

    // Blend space data array
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TArray<FAnim_BlendSpaceData> BlendSpaces;

    // Current active blend space
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blend Spaces")
    int32 ActiveBlendSpaceIndex;

    // Blend space control functions
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetActiveBlendSpace(EAnim_BlendSpaceType BlendSpaceType);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void UpdateBlendSpaceParameters(float XValue, float YValue);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void SetBlendSpaceWeight(EAnim_BlendSpaceType BlendSpaceType, float Weight);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    FAnim_BlendSpaceData GetActiveBlendSpaceData() const;

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    bool IsBlendSpaceActive(EAnim_BlendSpaceType BlendSpaceType) const;

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void AddBlendSpace(EAnim_BlendSpaceType BlendSpaceType, class UBlendSpace* BlendSpaceAsset);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void AddBlendSpace1D(EAnim_BlendSpaceType BlendSpaceType, class UBlendSpace1D* BlendSpace1DAsset);

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void RemoveBlendSpace(EAnim_BlendSpaceType BlendSpaceType);

    // Automatic parameter calculation
    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void CalculateLocomotionParameters();

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void CalculateCombatParameters();

    UFUNCTION(BlueprintCallable, Category = "Blend Space")
    void CalculateEmotionalParameters(float FearLevel, float HealthPercentage, float StaminaPercentage);

    // Blueprint events
    UFUNCTION(BlueprintImplementableEvent, Category = "Blend Space Events")
    void OnBlendSpaceChanged(EAnim_BlendSpaceType OldType, EAnim_BlendSpaceType NewType);

    UFUNCTION(BlueprintImplementableEvent, Category = "Blend Space Events")
    void OnBlendSpaceParametersUpdated(float XValue, float YValue);

private:
    // Cached references
    UPROPERTY()
    class ACharacter* OwnerCharacter;

    UPROPERTY()
    class UCharacterMovementComponent* MovementComponent;

    UPROPERTY()
    class UAnimInstance* AnimInstance;

    // Internal state
    float SmoothingSpeed;
    float ParameterSmoothingTime;
    FVector2D PreviousParameters;
    FVector2D TargetParameters;

    // Helper functions
    void CacheComponents();
    int32 FindBlendSpaceIndex(EAnim_BlendSpaceType BlendSpaceType) const;
    void SmoothBlendSpaceParameters(float DeltaTime);
    FVector2D CalculateDirectionalMovement() const;
    float CalculateMovementSpeed() const;
    void InitializeDefaultBlendSpaces();
};