#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist,
    DinosaurHerbivore,
    DinosaurCarnivore,
    DinosaurPredator,
    DinosaurAmbush
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Alert,
    Fearful,
    Panicked,
    Curious,
    Aggressive,
    Protective
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CautionLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ConfidenceLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState CurrentEmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, float> PersonalityTraits;
};

UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAnimationSystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAnimationSystemManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation Profile")
    FCharacterAnimationProfile AnimationProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* InteractionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* CombatDatabase;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void UpdateEmotionalState(EEmotionalState NewState, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void ApplyPersonalityModifier(const FString& TraitName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    float GetMovementStyleMultiplier() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    bool ShouldUseCautiousMovement() const;

private:
    UPROPERTY()
    float EmotionalStateIntensity = 1.0f;

    UPROPERTY()
    float StateTransitionTimer = 0.0f;

    void UpdateAnimationParameters();
    void ProcessEmotionalTransitions(float DeltaTime);
};