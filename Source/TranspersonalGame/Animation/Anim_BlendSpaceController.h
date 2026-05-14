#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Animation/BlendSpace.h"
#include "Animation/BlendSpace1D.h"
#include "Components/ActorComponent.h"
#include "Anim_BlendSpaceController.generated.h"

UENUM(BlueprintType)
enum class EAnim_BlendSpaceType : uint8
{
    Locomotion      UMETA(DisplayName = "Locomotion"),
    Combat          UMETA(DisplayName = "Combat"),
    Interaction     UMETA(DisplayName = "Interaction"),
    Emotion         UMETA(DisplayName = "Emotion")
};

USTRUCT(BlueprintType)
struct FAnim_BlendSpaceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Space")
    UBlendSpace* BlendSpace;

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
        BlendSpace = nullptr;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Blend Spaces")
    TMap<EAnim_BlendSpaceType, FAnim_BlendSpaceData> BlendSpaces;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementDirection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float LeanAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float CombatStance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float WeaponType;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Blend Space Control")
    void SetBlendSpaceValues(EAnim_BlendSpaceType BlendSpaceType, float XValue, float YValue, float Weight = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Blend Space Control")
    void ActivateBlendSpace(EAnim_BlendSpaceType BlendSpaceType, bool bActivate);

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void UpdateMovementBlendSpace(float Speed, float Direction);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateCombatBlendSpace(float Stance, float Weapon);

    UFUNCTION(BlueprintPure, Category = "Blend Space Control")
    FAnim_BlendSpaceData GetBlendSpaceData(EAnim_BlendSpaceType BlendSpaceType) const;

    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetNormalizedMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "Movement")
    float GetMovementDirection() const;

private:
    void InitializeDefaultBlendSpaces();
    void UpdateBlendSpaceWeights(float DeltaTime);
    
    UPROPERTY()
    float BlendSpaceUpdateRate;

    UPROPERTY()
    float LastUpdateTime;
};