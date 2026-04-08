#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/AnimInstance.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"
#include "../Core/AnimationSystemManager.h"
#include "MotionMatchingController.generated.h"

USTRUCT(BlueprintType)
struct FMotionMatchingDatabase
{
    GENERATED_BODY()

    // Base de dados para diferentes estados
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* LocomotionDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* IdleDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* AlertDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* FearDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* AggressiveDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* FeedingDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    class UPoseSearchDatabase* RestingDatabase;
};

USTRUCT(BlueprintType)
struct FPersonalityAnimationModifiers
{
    GENERATED_BODY()

    // Modificadores aplicados às animações baseados na personalidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "2.0"))
    float SpeedMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.8", ClampMax = "1.2"))
    float StepFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NervousTwitchChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HeadLookAroundChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.5", ClampMax = "1.5"))
    float BodyTensionLevel = 1.0f;
};

/**
 * Controlador do sistema Motion Matching
 * Seleciona animações baseadas no estado emocional e personalidade
 */
UCLASS(ClassGroup=(Animation), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMotionMatchingController : public UActorComponent
{
    GENERATED_BODY()

public:
    UMotionMatchingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configuração das bases de dados
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesPlayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesSmallHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesLargeHerbivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesSmallCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesLargeCarnivore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    FMotionMatchingDatabase DatabasesApex;

    // Schema de busca
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motion Matching")
    class UPoseSearchSchema* SearchSchema;

    // Referência ao sistema de animação
    UPROPERTY(BlueprintReadOnly, Category = "Animation System")
    class UAnimationSystemManager* AnimationManager;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    class UPoseSearchDatabase* CurrentDatabase;

    UPROPERTY(BlueprintReadOnly, Category = "Motion Matching")
    FPersonalityAnimationModifiers CurrentModifiers;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void UpdateDatabaseSelection();

    UFUNCTION(BlueprintCallable, Category = "Motion Matching")
    void ApplyPersonalityModifiers();

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    class UPoseSearchDatabase* GetOptimalDatabase() const;

    UFUNCTION(BlueprintPure, Category = "Motion Matching")
    float GetBlendTime() const;

    // Eventos para Animation Blueprint
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDatabaseChanged, class UPoseSearchDatabase*, NewDatabase);
    UPROPERTY(BlueprintAssignable)
    FOnDatabaseChanged OnDatabaseChanged;

private:
    // Estado interno
    float DatabaseUpdateTimer;
    float ModifierUpdateTimer;
    class UPoseSearchDatabase* PreviousDatabase;

    // Funções internas
    FMotionMatchingDatabase* GetDatabaseSetForCharacterType(ECharacterAnimationType CharacterType);
    class UPoseSearchDatabase* SelectDatabaseForEmotionalState(const FMotionMatchingDatabase& DatabaseSet, EEmotionalState EmotionalState) const;
    void CalculatePersonalityModifiers();
    void SmoothDatabaseTransition();
};