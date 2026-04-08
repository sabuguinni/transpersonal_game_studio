#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Animation/AnimInstance.h"
#include "Components/ActorComponent.h"
#include "AnimationSystemManager.generated.h"

UENUM(BlueprintType)
enum class ECharacterArchetype : uint8
{
    Protagonist_Scientist   UMETA(DisplayName = "Scientist Protagonist"),
    Herbivore_Small        UMETA(DisplayName = "Small Herbivore"),
    Herbivore_Large        UMETA(DisplayName = "Large Herbivore"),
    Carnivore_Small        UMETA(DisplayName = "Small Carnivore"),
    Carnivore_Medium       UMETA(DisplayName = "Medium Carnivore"),
    Carnivore_Large        UMETA(DisplayName = "Large Carnivore"),
    Carnivore_Apex         UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Alert          UMETA(DisplayName = "Alert"),
    Nervous        UMETA(DisplayName = "Nervous"),
    Fearful        UMETA(DisplayName = "Fearful"),
    Panicked       UMETA(DisplayName = "Panicked"),
    Aggressive     UMETA(DisplayName = "Aggressive"),
    Hunting        UMETA(DisplayName = "Hunting"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Resting        UMETA(DisplayName = "Resting"),
    Curious        UMETA(DisplayName = "Curious")
};

USTRUCT(BlueprintType)
struct FCharacterAnimationProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Identity")
    ECharacterArchetype Archetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Identity")
    FString IndividualID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Traits")
    float MovementSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Traits")
    float StepLength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Traits")
    float HeadBobIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement Traits")
    float ShoulderSwayAmount = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Confidence = 0.5f; // 0.0 = Very Timid, 1.0 = Very Bold

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float Alertness = 0.5f; // 0.0 = Oblivious, 1.0 = Hyper-Alert

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Weight = 1.0f; // Affects how heavy movements feel

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physical Traits")
    float Agility = 1.0f; // Affects reaction speed and fluidity

    FCharacterAnimationProfile()
    {
        Archetype = ECharacterArchetype::Protagonist_Scientist;
        IndividualID = TEXT("Default");
        MovementSpeed = 1.0f;
        StepLength = 1.0f;
        HeadBobIntensity = 1.0f;
        ShoulderSwayAmount = 1.0f;
        Confidence = 0.5f;
        Alertness = 0.5f;
        Weight = 1.0f;
        Agility = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* IdleDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* AlertDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchDatabase* FearDatabase;

    // Core animation functions
    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void SetEmotionalState(EEmotionalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void UpdateMovementParameters(float Speed, FVector Direction, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    void ApplyIndividualVariation();

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    float GetWeightedAnimationSpeed() const;

    UFUNCTION(BlueprintCallable, Category = "Animation System")
    FVector GetModifiedStepVector(FVector BaseStep) const;

private:
    float CurrentSpeed;
    FVector CurrentDirection;
    float StateTransitionTime;
    float TimeSinceLastStateChange;

    void ProcessEmotionalStateTransition(float DeltaTime);
    void CalculateMovementModifiers();
};