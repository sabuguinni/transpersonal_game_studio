#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Engine/Engine.h"
#include "Anim_BlendSpaceController.generated.h"

UENUM(BlueprintType)
enum class EAnim_BlendSpaceType : uint8
{
    Locomotion      UMETA(DisplayName = "Locomotion"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Emotional       UMETA(DisplayName = "Emotional")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* BlendSpace2D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace1D* BlendSpace1D;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    FVector2D CurrentInput;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    float BlendWeight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    bool bIsActive;

    FAnim_BlendSpaceData()
    {
        BlendSpace2D = nullptr;
        BlendSpace1D = nullptr;
        CurrentInput = FVector2D::ZeroVector;
        BlendWeight = 0.0f;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TMap<EAnim_BlendSpaceType, FAnim_BlendSpaceData> BlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float BlendSpeedMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float InputSmoothingSpeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    EAnim_BlendSpaceType CurrentBlendSpaceType;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
    FVector2D SmoothedInput;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpaceInput(EAnim_BlendSpaceType BlendSpaceType, const FVector2D& Input);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetBlendSpace1DInput(EAnim_BlendSpaceType BlendSpaceType, float Input);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SwitchToBlendSpace(EAnim_BlendSpaceType NewBlendSpaceType);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterBlendSpace2D(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace* BlendSpace);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void RegisterBlendSpace1D(EAnim_BlendSpaceType BlendSpaceType, UBlendSpace1D* BlendSpace);

    UFUNCTION(BlueprintPure, Category = "Animation")
    FVector2D GetCurrentBlendSpaceInput(EAnim_BlendSpaceType BlendSpaceType) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    float GetBlendSpaceWeight(EAnim_BlendSpaceType BlendSpaceType) const;

    UFUNCTION(BlueprintPure, Category = "Animation")
    bool IsBlendSpaceActive(EAnim_BlendSpaceType BlendSpaceType) const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateLocomotionBlendSpace(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateCombatBlendSpace(float AttackIntensity, float DefenseStance);

private:
    void SmoothInputTransition(float DeltaTime);
    void UpdateBlendWeights(float DeltaTime);
    void ValidateBlendSpaces();
};